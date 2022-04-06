#include "decision-graph/models/Query.hpp"
#include "decision-graph/models/UserLabelsModel.hpp"
#include "decision-graph/parsers/QueryParser.y.hpp"
#include "decision-graph/parsers/QueryScanner.lex.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"
#include "rfcommon/hash40.hpp"
#include <cstdio>
#include <cinttypes>
#include <memory>

// ----------------------------------------------------------------------------
Matcher Matcher::start()
{
    return Matcher(
        0,
        0,
        0,
        0
    );
}

// ----------------------------------------------------------------------------
Matcher Matcher::wildCard(uint8_t hitFlags)
{
    return Matcher(
        0,
        0,
        hitFlags,
        0
    );
}

// ----------------------------------------------------------------------------
Matcher Matcher::motion(rfcommon::FighterMotion motion, uint8_t hitFlags)
{
    return Matcher(
        motion,
        0,
        hitFlags,
        MATCH_MOTION
    );
}

// ----------------------------------------------------------------------------
Matcher::Matcher(
        rfcommon::FighterMotion motion,
        rfcommon::FighterStatus status,
        uint8_t hitType,
        uint8_t matchFlags)
    : motion_(motion)
    , status_(status)
    , hitFlags_(hitType)
    , matchFlags_(matchFlags)
{}

// ----------------------------------------------------------------------------
bool Matcher::matches(const State& state) const
{
    if (!!(matchFlags_ & MATCH_STATUS))
        if (state.status() != status_)
            return false;

    if (!!(matchFlags_ & MATCH_MOTION))
        if (state.motion() != motion_)
            return false;

    if (hitFlags_)
    {
        bool onHit = state.opponentInHitlag();
        bool onShield = state.opponentInShieldlag();
        bool onWhiff = !state.opponentInHitlag() && !state.opponentInShieldlag();
        uint8_t compare =
                (static_cast<uint8_t>(onHit) << 0)
              | (static_cast<uint8_t>(onWhiff) << 1)
              | (static_cast<uint8_t>(onShield) << 2);
        if (!(hitFlags_ & compare))
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
QueryASTNode* Query::parse(const char* text)
{
    qpscan_t scanner;
    qppstate* parser;
    YY_BUFFER_STATE buf;
    QPSTYPE pushed_value;
    int pushed_char;
    int parse_result;
    QueryASTNode* ast = nullptr;

    if (qplex_init(&scanner) != 0)
        goto init_scanner_failed;
    buf = qp_scan_bytes(text, strlen(text), scanner);
    if (buf == nullptr)
        goto scan_bytes_failed;
    parser = qppstate_new();
    if (parser == nullptr)
        goto init_parser_failed;

    do
    {
        pushed_char = qplex(&pushed_value, scanner);
        parse_result = qppush_parse(parser, pushed_char, &pushed_value, &ast);
    } while (parse_result == YYPUSH_MORE);

    qppstate_delete(parser);
    qp_delete_buffer(buf, scanner);
    qplex_destroy(scanner);

    if (parse_result == 0)
        return ast;
    if (ast)
        QueryASTNode::destroyRecurse(ast);
    return nullptr;

    init_parser_failed  : qp_delete_buffer(buf, scanner);
    scan_bytes_failed   : qplex_destroy(scanner);
    init_scanner_failed : return nullptr;
}

// ----------------------------------------------------------------------------
struct Fragment
{
    rfcommon::SmallVector<int, 4> in;
    rfcommon::SmallVector<int, 4> out;
    bool bridge = false;
};

// ----------------------------------------------------------------------------
static void duplicateMatchers(int idx, rfcommon::Vector<Matcher>* matchers, rfcommon::HashMap<int, int>* indexMap)
{
    if (indexMap->insertNew(idx, matchers->count()) == indexMap->end())
        return;

    matchers->push(Matcher(matchers->at(idx)));
    for (int i : matchers->at(idx).next)
        duplicateMatchers(i, matchers, indexMap);
}
static Fragment duplicateFragment(const Fragment& f, rfcommon::Vector<Matcher>* matchers)
{
    Fragment dup;
    rfcommon::HashMap<int, int> indexMap;  // Map indices of old matchers to the newly inserted matchers
    for (int i : f.in)
        duplicateMatchers(i, matchers, &indexMap);

    // Fix transitions
    for (auto kv : indexMap)
    {
        Matcher& newMatcher = matchers->at(kv.value());
        for (int& i : newMatcher.next)
            i = indexMap.find(i)->value();
    }

    // Fragment inputs/outputs
    for (int i : f.in)
        dup.in.push(indexMap.find(i)->value());
    for (int i : f.out)
        dup.out.push(indexMap.find(i)->value());

    return dup;
}

// ----------------------------------------------------------------------------
static bool compileASTRecurse(
        const QueryASTNode* node,
        const UserLabelsModel* userLabels,
        rfcommon::FighterID fighterID,
        rfcommon::Vector<Matcher>* matchers,
        rfcommon::SmallVector<Fragment, 16>* fstack,
        rfcommon::SmallVector<uint8_t, 16>* qstack)
{
    switch (node->type)
    {
    case QueryASTNode::STATEMENT: {
        if (!compileASTRecurse(node->statement.child, userLabels, fighterID, matchers, fstack, qstack)) return false;
        if (!compileASTRecurse(node->statement.next, userLabels, fighterID, matchers, fstack, qstack)) return false;
        if (fstack->count() < 2) return false;

        Fragment& right = fstack->back(1);
        Fragment& left = fstack->back(2);
        for (int o : left.out)
            for (int i : right.in)
                matchers->at(o).next.push(i);

        if (right.bridge)
            left.out.push(std::move(right.out));
        else
            left.out = std::move(right.out);

        if (left.bridge)
        {
            for (int i : right.in)
                left.in.push(i);
            left.bridge = false;
        }

        fstack->pop();
    } break;

    case QueryASTNode::REPITITION: {
        if (!compileASTRecurse(node->repitition.child, userLabels, fighterID, matchers, fstack, qstack)) return false;
        if (fstack->count() < 1) return false;

        // Invalid values
        if (node->repitition.minreps < 0)
            return false;

        Fragment& f = fstack->back();

        // Mark the entire fragment as optional if minreps or maxreps is 0
        if (node->repitition.minreps == 0 || node->repitition.maxreps == 0)
            f.bridge = true;

        if (node->repitition.maxreps == -1)
        {
            // We will need min-1 duplicates of the current fragment to implement the
            // repitition logic
            rfcommon::SmallVector<Fragment, 8> fragments;
            for (int n = 1; n < node->repitition.minreps; ++n)
                fragments.push(duplicateFragment(f, matchers));

            // Add repeat to fragment
            for (int a : f.out)
                for (int b : f.in)
                    matchers->at(a).next.push(b);

            // Wire up outputs among duplicates
            for (int n = 1; n < fragments.count(); ++n)
            {
                for (int o : fragments[n].out)
                    for (int i : fragments[n-1].in)
                        matchers->at(o).next.push(i);
            }

            if (fragments.count() > 0)
            {
                for (int i : f.in)
                    for (int o : fragments[0].out)
                        matchers->at(o).next.push(i);
                f.in = std::move(fragments.back().in);
            }
        }
        else
        {
            // Invalid values
            if (node->repitition.maxreps < 0 || node->repitition.minreps > node->repitition.maxreps)
                return false;

            // Special case if maxreps is 0, remove all connections
            if (node->repitition.maxreps == 0)
            {
                f.in.clear();
                f.out.clear();
                break;
            }

            // Nothing to do
            if (node->repitition.maxreps == 1)
                break;

            // We will need max-1 duplicates of the current fragment to implement the
            // repitition logic
            rfcommon::SmallVector<Fragment, 8> fragments;
            for (int n = 1; n != node->repitition.maxreps; ++n)
                fragments.push(duplicateFragment(f, matchers));

            // Wire up outputs among duplicates
            for (int n = 1; n != fragments.count(); ++n)
            {
                for (int o : fragments[n].out)
                    for (int i : fragments[n-1].in)
                        matchers->at(o).next.push(i);
            }

            // Wire up outputs to original fragment
            for (int o : fragments[0].out)
                for (int i : f.in)
                    matchers->at(o).next.push(i);

            // Wire up inputs to original fragment
            if (node->repitition.minreps > 1)
                f.in.clear();
            for (int n = std::max(0, node->repitition.minreps - 2); n != node->repitition.maxreps - 1; ++n)
            {
                for (int i : fragments[n].in)
                    f.in.push(i);
            }
        }
    } break;

    case QueryASTNode::UNION: {
        if (!compileASTRecurse(node->union_.child, userLabels, fighterID, matchers, fstack, qstack)) return false;
        if (!compileASTRecurse(node->union_.next, userLabels, fighterID, matchers, fstack, qstack)) return false;
        if (fstack->count() < 2) return false;

        Fragment& f1 = fstack->back(1);
        Fragment& f2 = fstack->back(2);

        f2.in.push(std::move(f1.in));
        f2.out.push(std::move(f1.out));
        f2.bridge = (f1.bridge || f2.bridge);

        fstack->pop();
    } break;

    case QueryASTNode::INVERSION:
        if (!compileASTRecurse(node->inversion.child, userLabels, fighterID, matchers, fstack, qstack)) return false;
        break;

    case QueryASTNode::WILDCARD: {
        uint8_t hitFlags = 0;
        if (qstack->count() > 0)
        {
            if (!!(qstack->back() & QueryASTNode::QualifierFlags::QUAL_HIT))
                hitFlags |= Matcher::HIT;
            if (!!(qstack->back() & QueryASTNode::QualifierFlags::QUAL_WHIFF))
                hitFlags |= Matcher::WHIFF;
            if (!!(qstack->back() & QueryASTNode::QualifierFlags::QUAL_OS))
                hitFlags |= Matcher::ON_SHIELD;
        }

        fstack->push({{matchers->count()}, {matchers->count()}});
        matchers->push(Matcher::wildCard(hitFlags));
    } break;

    case QueryASTNode::LABEL: {
        uint8_t hitFlags = 0;
        if (qstack->count() > 0)
        {
            if (!!(qstack->back() & QueryASTNode::QualifierFlags::QUAL_HIT))
                hitFlags |= Matcher::HIT;
            if (!!(qstack->back() & QueryASTNode::QualifierFlags::QUAL_WHIFF))
                hitFlags |= Matcher::WHIFF;
            if (!!(qstack->back() & QueryASTNode::QualifierFlags::QUAL_OS))
                hitFlags |= Matcher::ON_SHIELD;
        }

        // Assume label is a user label and maps to one more more motion
        // values
        auto motions = userLabels->userLabelToMotion(node->label.cStr(), fighterID);
        if (motions.count() > 0)
        {
            Fragment& fragment = fstack->emplace();
            for (const auto& motion : motions)
            {
                fragment.in.push(matchers->count());
                fragment.out.push(matchers->count());
                matchers->push(Matcher::motion(motion, hitFlags));
            }

            // If the user label maps to multiple motions a, b, c, then
            // connect them such that it matches [abc]+
            if (motions.count() > 1)
            {
                for (int a = 1; a <= motions.count(); ++a)
                    for (int b = 1; b <= motions.count(); ++b)
                        matchers->back(a).next.push(matchers->count() - b);
            }
            break;
        }

        // Assume label is actually a label and maps to a single motion
        // value
        auto motion = userLabels->labelToMotion(node->label.cStr());
        if (motion.value() > 0)
        {
            fstack->push({{matchers->count()}, {matchers->count()}});
            matchers->push(Matcher::motion(motion, hitFlags));
            break;
        }

        return false;
    } break;

    case QueryASTNode::QUALIFIER: {
        qstack->push(node->qualifier.flags);
        if (!compileASTRecurse(node->qualifier.child, userLabels, fighterID, matchers, fstack, qstack)) return false;
        qstack->pop();
        if (fstack->count() < 1) return false;
    } break;
    }

    return true;
}

// ----------------------------------------------------------------------------
Query* Query::compileAST(const QueryASTNode* ast, const UserLabelsModel* userLabels, rfcommon::FighterID fighterID)
{
    std::unique_ptr<Query> query(new Query);
    query->matchers_.push(Matcher::start());

    rfcommon::SmallVector<Fragment, 16> fstack;
    rfcommon::SmallVector<uint8_t, 16> qstack;
    if (!compileASTRecurse(ast, userLabels, fighterID, &query->matchers_, &fstack, &qstack))
        return nullptr;
    if (fstack.count() != 1)
        return nullptr;

    // Patch in starting matcher, which is always at index 0
    for (int i : fstack[0].in)
        query->matchers_[0].next.push(i);

    // Mark all matchers with dangling outgoing transitions with stop condition
    for (int i : fstack[0].out)
        query->matchers_[i].setStop();

    // Removes duplicate state transitions
    for (Matcher& matcher : query->matchers_)
    {
        try_again:
        for (int a = 0; a != matcher.next.count(); ++a)
            for (int b = a + 1; b < matcher.next.count(); ++b)
                if (matcher.next[a] == matcher.next[b])
                {
                    matcher.next.erase(b);
                    goto try_again;
                }
    }

    return query.release();
}

// ----------------------------------------------------------------------------
rfcommon::Vector<SequenceRange> Query::apply(const Sequence& seq)
{
    struct MatchInfo
    {
        int lastList;
    };

    rfcommon::Vector<SequenceRange> result;
    rfcommon::SmallVector<int, 16> l1, l2;
    rfcommon::SmallVector<MatchInfo, 16> info(matchers_.count());

    // Returns the ending index in the sequence (exclusive) for the
    // current search pattern. If no pattern was found then this returns
    // the starting index
    auto doSequenceMatch = [this, &seq, &l1, &l2, &info](const int startIdx) -> int {
        int stateIdx = startIdx;

        // Prepare current and next state lists
        int listid = 0;
        rfcommon::SmallVector<int, 16>* clist = &l1;
        rfcommon::SmallVector<int, 16>* nlist = &l2;
        clist->clear();
        for (int i : matchers_[0].next)
            clist->emplace(i);

        memset(info.data(), 0, sizeof(int) * matchers_.count());

        while (true)
        {
            listid++;
            nlist->clear();
            for (int matcherIdx : *clist)
                if (matchers_[matcherIdx].matches(seq.states[stateIdx]))
                {
                    for (int nextMatcherIdx : matchers_[matcherIdx].next)
                        if (info[nextMatcherIdx].lastList != listid)
                        {
                            info[nextMatcherIdx].lastList = listid;
                            nlist->push(nextMatcherIdx);
                        }

                    if (stateIdx + 1 >= seq.states.count())
                        return stateIdx + 1;

                    if (matchers_[matcherIdx].isStop())
                    {
                        for (int nextMatcherIdx : matchers_[matcherIdx].next)
                            if (matchers_[nextMatcherIdx].matches(seq.states[stateIdx + 1]))
                                goto skip_return;

                        return stateIdx + 1;
                        skip_return:;
                    }
                }

            if (nlist->count() == 0 || stateIdx + 1 >= seq.states.count())
                return startIdx;  // Failed to match anything

            // Advance
            stateIdx++;
            std::swap(clist, nlist);
        }
    };

    // Nothing to do
    if (matchers_.count() == 0 || matchers_[0].next.count() == 0)
        return result;

    // We search the sequence of states rather than the graph, because we are
    // interested in matching sequences of decisions
    for (int stateIdx = 0; stateIdx != seq.states.count(); ++stateIdx)
    {
        const int stateEndIdx = doSequenceMatch(stateIdx);
        if (stateEndIdx > stateIdx)
            result.emplace(stateIdx, stateEndIdx);
    }

    return result;
}

// ----------------------------------------------------------------------------
void Query::exportDOT(const char* filename, const UserLabelsModel* userLabels, rfcommon::FighterID fighterID)
{
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "digraph query {\n");

    for (int i = 0; i != matchers_.count(); ++i)
    {
        const char* label =
                i == 0 ? "start" :
                matchers_[i].isWildcard() ? "." :
                userLabels->motionToLabel(matchers_[i].motion_);
        const char* color = matchers_[i].isStop() ? "red" : "black";
        if (label)
            fprintf(fp, "m%d [shape=\"record\",color=\"%s\",label=\"%s", i, color, label);
        else
            fprintf(fp, "m%d [shape=\"record\",color=\"%s\",label=\"%" PRIu64,
                    i, color, matchers_[i].motion_.value());

        if (matchers_[i].hitFlagSet(Matcher::HIT))
            fprintf(fp, " | HIT");
        if (matchers_[i].hitFlagSet(Matcher::WHIFF))
            fprintf(fp, " | WHIFF");
        if (matchers_[i].hitFlagSet(Matcher::ON_SHIELD))
            fprintf(fp, " | OS");
        fprintf(fp, "\"];\n");
    }

    for (int i = 0; i != matchers_.count(); ++i)
    {
        for (int e : matchers_[i].next)
            fprintf(fp, "m%d -> m%d;\n", i, e);
    }

    fprintf(fp, "}\n");
    fclose(fp);
}
