#include "decision-graph/models/Query.hpp"
#include "decision-graph/parsers/QueryParser.y.hpp"
#include "decision-graph/parsers/QueryScanner.lex.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"

#include "rfcommon/HashMap.hpp"
#include "rfcommon/MotionLabels.hpp"

#include <cstdio>
#include <cinttypes>
#include <memory>

// ----------------------------------------------------------------------------
Matcher Matcher::start()
{
    return Matcher(
        rfcommon::FighterMotion::makeInvalid(),
        rfcommon::FighterStatus::makeInvalid(),
        0,
        0
    );
}

// ----------------------------------------------------------------------------
Matcher Matcher::wildCard(uint8_t ctxQualFlags)
{
    return Matcher(
        rfcommon::FighterMotion::makeInvalid(),
        rfcommon::FighterStatus::makeInvalid(),
        ctxQualFlags,
        0
    );
}

// ----------------------------------------------------------------------------
Matcher Matcher::motion(rfcommon::FighterMotion motion, uint8_t ctxQualFlags)
{
    return Matcher(
        motion,
        rfcommon::FighterStatus::makeInvalid(),
        ctxQualFlags,
        MATCH_MOTION
    );
}

// ----------------------------------------------------------------------------
Matcher::Matcher(
        rfcommon::FighterMotion motion,
        rfcommon::FighterStatus status,
        uint8_t ctxQualFlags,
        uint8_t matchFlags)
    : motion_(motion)
    , status_(status)
    , ctxQualFlags_(ctxQualFlags)
    , matchFlags_(matchFlags)
{}

// ----------------------------------------------------------------------------
bool Matcher::matches(const State& state) const
{
    if (!!(matchFlags_ & MATCH_STATUS))
        if (state.status != status_)
            return false;

    if (!!(matchFlags_ & MATCH_MOTION))
        if (state.motion != motion_)
            return false;

    if (ctxQualFlags_)
    {
        bool onHit = state.opponentInHitlag();
        bool onShield = state.opponentInShieldlag();
        bool onWhiff = !state.opponentInHitlag() && !state.opponentInShieldlag();
        uint8_t compare =
                (static_cast<uint8_t>(onHit) << 0)
              | (static_cast<uint8_t>(onWhiff) << 1)
              | (static_cast<uint8_t>(onShield) << 2);
        if (!(ctxQualFlags_ & compare))
            return false;
    }

    return true;
}

// ----------------------------------------------------------------------------
QueryASTNode* Query::parse(const rfcommon::String& text)
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
    buf = qp_scan_bytes(text.cStr(), text.length(), scanner);
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
/*
 * Represents a subset of the final finite automaton.
 *
 * Each fragment holds a list of unresolved incoming and outgoing transitions.
 * As the AST is compiled, fragments are assembled together by connecting these
 * transitions to each other in order to form larger and larger fragments, until
 * the final finite automaton is completed.
 *
 * The "in" and "out" lists are indices into the vector of states/matchers.
 * The "bridge" flag handles a special case where a fragment can have a direct
 * transition from input to output without going through an internal state/matcher.
 */
struct Fragment
{
    rfcommon::SmallVector<int, 4> in;
    rfcommon::SmallVector<int, 4> out;
    bool bridge = false;
};

// ----------------------------------------------------------------------------
static void duplicateMatchers(int idx, rfcommon::Vector<Matcher>* matchers, rfcommon::HashMap<int, int>* indexMap)
{
    if (indexMap->insertIfNew(idx, matchers->count()) == indexMap->end())
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
        const rfcommon::MotionLabels* labels,
        rfcommon::FighterID fighterID,
        rfcommon::Vector<Matcher>* matchers,
        rfcommon::Vector<rfcommon::SmallVector<rfcommon::FighterMotion, 4>>* mergeMotions,
        rfcommon::SmallVector<Fragment, 16>* fstack,
        rfcommon::SmallVector<uint8_t, 16>* qstack)
{
    // OR all flags currently on the stack
    auto calcContextQualifierFlags = [](rfcommon::SmallVector<uint8_t, 16>* qstack) -> uint8_t {
        uint8_t contextQualifierFlags = 0;
        for (uint8_t flag : *qstack)
        {
            if (!!(flag & QueryASTNode::HIT))
                contextQualifierFlags |= Matcher::HIT;
            if (!!(flag & QueryASTNode::WHIFF))
                contextQualifierFlags |= Matcher::WHIFF;
            if (!!(flag & QueryASTNode::OS))
                contextQualifierFlags |= Matcher::SHIELD;
        }
        return contextQualifierFlags;
    };

    switch (node->type)
    {
    case QueryASTNode::STATEMENT: {
        if (!compileASTRecurse(node->statement.child, labels, fighterID, matchers, mergeMotions, fstack, qstack)) return false;
        if (!compileASTRecurse(node->statement.next, labels, fighterID, matchers, mergeMotions, fstack, qstack)) return false;
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
        if (!compileASTRecurse(node->repitition.child, labels, fighterID, matchers, mergeMotions, fstack, qstack)) return false;
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
        if (!compileASTRecurse(node->union_.child, labels, fighterID, matchers, mergeMotions, fstack, qstack)) return false;
        if (!compileASTRecurse(node->union_.next, labels, fighterID, matchers, mergeMotions, fstack, qstack)) return false;
        if (fstack->count() < 2) return false;

        Fragment& f1 = fstack->back(1);
        Fragment& f2 = fstack->back(2);

        f2.in.push(std::move(f1.in));
        f2.out.push(std::move(f1.out));
        f2.bridge = (f1.bridge || f2.bridge);

        fstack->pop();
    } break;

    case QueryASTNode::INVERSION:
        if (!compileASTRecurse(node->inversion.child, labels, fighterID, matchers, mergeMotions, fstack, qstack)) return false;
        break;

    case QueryASTNode::WILDCARD: {
        fstack->push({{matchers->count()}, {matchers->count()}});
        matchers->push(Matcher::wildCard(calcContextQualifierFlags(qstack)));
    } break;

    case QueryASTNode::LABEL: {
        uint8_t ctxtQualFlags = calcContextQualifierFlags(qstack);

        // Assume label is a user label and maps to one or more motion
        // values
        auto motions = labels->toMotions(fighterID, node->labels.label.cStr());
        if (motions.count() > 0)
        {
            Fragment& fragment = fstack->emplace();
            for (const auto& motion : motions)
            {
                fragment.in.push(matchers->count());
                fragment.out.push(matchers->count());
                matchers->push(Matcher::motion(motion, ctxtQualFlags));
            }

            // If the user label maps to multiple motions a, b, c, then
            // connect them such that it matches [abc]+
            if (motions.count() > 1)
            {
                for (int a = 1; a <= motions.count(); ++a)
                    for (int b = 1; b <= motions.count(); ++b)
                        matchers->back(a).next.push(matchers->count() - b);
            }

            // Store list of motions so they can be used to merge states in
            // matched sequences
            mergeMotions->push(std::move(motions));
            break;
        }

        // Assume label is actually a hash40 string and maps to a single motion
        // value
        auto motion = labels->toMotion(node->labels.label.cStr());
        if (motion.isValid())
        {
            fstack->push({{matchers->count()}, {matchers->count()}});
            matchers->push(Matcher::motion(motion, ctxtQualFlags));
            break;
        }

        // Assume string is hex describing a hash40 motion value
        motion = rfcommon::FighterMotion::fromHexString(node->labels.label.cStr());
        if (motion.isValid())
        {
            fstack->push({{matchers->count()}, {matchers->count()}});
            matchers->push(Matcher::motion(motion, ctxtQualFlags));
            break;
        }

        return false;
    } break;

    case QueryASTNode::CONTEXT_QUALIFIER: {
        qstack->push(node->contextQualifier.flags);
        if (!compileASTRecurse(node->contextQualifier.child, labels, fighterID, matchers, mergeMotions, fstack, qstack)) return false;
        qstack->pop();
    } break;
    }

    return true;
}

// ----------------------------------------------------------------------------
Query* Query::compileAST(const QueryASTNode* ast, const rfcommon::MotionLabels* labels, rfcommon::FighterID fighterID)
{
    std::unique_ptr<Query> query(new Query);
    query->matchers_.push(Matcher::start());

    rfcommon::SmallVector<Fragment, 16> fstack;  // "fragment stack"
    rfcommon::SmallVector<uint8_t, 16> qstack;   // "qualifier stack"

    if (!compileASTRecurse(ast, labels, fighterID, &query->matchers_, &query->mergeMotions_, &fstack, &qstack))
        return nullptr;
    if (fstack.count() != 1)
        return nullptr;

    // Patch in starting matcher, which is always at index 0
    for (int i : fstack[0].in)
        query->matchers_[0].next.push(i);

    // Mark all matchers with dangling outgoing transitions with the accept condition
    for (int i : fstack[0].out)
        query->matchers_[i].setAcceptCondition();

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
rfcommon::Vector<Range> Query::apply(const States& states, const Range& range) const
{
    struct MatchInfo
    {
        int lastList;
    };

    rfcommon::Vector<Range> result;
    rfcommon::SmallVector<int, 16> l1, l2;
    auto info = rfcommon::SmallVector<MatchInfo, 16>::makeResized(matchers_.count());

    // Returns the ending index in the sequence (exclusive) for the
    // current search pattern. If no pattern was found then this returns
    // the starting index
    auto doSequenceMatch = [this, &states, &l1, &l2, &info](const int startIdx) -> int {
        const int maxMatchLength = 50;
        int stateIdx = startIdx;

        // Prepare current and next state lists
        int listid = 0;
        auto* clist = &l1;
        auto* nlist = &l2;
        clist->clear();
        for (int i : matchers_[0].next)
            clist->emplace(i);

        memset(info.data(), 0, sizeof(int) * matchers_.count());

        while (true)
        {
            listid++;
            nlist->clear();
            for (int matcherIdx : *clist)
            {
                if (matchers_[matcherIdx].matches(states[stateIdx]) == false)
                    continue;

                for (int nextMatcherIdx : matchers_[matcherIdx].next)
                    if (info[nextMatcherIdx].lastList != listid)
                    {
                        info[nextMatcherIdx].lastList = listid;
                        nlist->push(nextMatcherIdx);
                    }

                // We have run out of states to match
                if (stateIdx + 1 >= states.count() || stateIdx >= startIdx + maxMatchLength)
                {
                    if (matchers_[matcherIdx].isAcceptCondition())
                        return stateIdx + 1;  // Success, return the "end" of the range which is the last index plus 1

                    // the state machine is not complete, which means
                    // we only have a partial match -> failure
                    return startIdx;
                }

                if (matchers_[matcherIdx].isAcceptCondition())
                {
                    for (int nextMatcherIdx : matchers_[matcherIdx].next)
                        if (matchers_[nextMatcherIdx].matches(states[stateIdx + 1]))
                            goto skip_return;

                    // Success, return the "end" of the range which is the last index plus 1
                    return stateIdx + 1;
                    skip_return:;
                }
            }

            if (nlist->count() == 0
                    || stateIdx >= startIdx + maxMatchLength
                    || stateIdx + 1 >= states.count())
            {
                return startIdx;  // Failed to match anything
            }

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
    for (int startIdx = range.startIdx; startIdx < range.endIdx; ++startIdx)
    {
        const int endIdx = doSequenceMatch(startIdx);
        if (endIdx > startIdx)
        {
            result.emplace(startIdx, endIdx);
            startIdx = endIdx - 1;
        }
    }

    return result;
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Sequence> Query::mergeMotions(const States& states, const rfcommon::Vector<Range>& matches) const
{
    rfcommon::Vector<Sequence> result;

    if (mergeMotions_.count() == 0)
    {
        for (const auto& range : matches)
        {
            Sequence& seq = result.emplace();
            for (int idx = range.startIdx; idx != range.endIdx; ++idx)
                seq.idxs.push(idx);
        }
        return result;
    }

    auto canMergeMotions = [this](rfcommon::FighterMotion m1, rfcommon::FighterMotion m2) -> bool {
        for (int group = 0; group != mergeMotions_.count(); ++group)
            if (mergeMotions_[group].findFirst(m1) != mergeMotions_[group].end()
                && mergeMotions_[group].findFirst(m2) != mergeMotions_[group].end())
            {
                return true;
            }
        return false;
    };

    for (const auto& range : matches)
    {
        auto& seq = result.emplace();
        seq.idxs.push(range.startIdx);
        for (int idx = range.startIdx + 1; idx < range.endIdx; ++idx)
        {
            if (canMergeMotions(states[idx-1].motion, states[idx].motion))
                continue;
            seq.idxs.push(idx);
        }
    }

    return result;
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Sequence> Query::normalizeMotions(const States& states, const rfcommon::Vector<Sequence>& matches) const
{
    if (mergeMotions_.count() == 0)
        return matches;

    rfcommon::Vector<Sequence> result;
    rfcommon::HashMap<rfcommon::FighterMotion, int, rfcommon::FighterMotion::Hasher> groupIdxMap;
    for (const auto& seq : matches)
    {
        auto& resultSeq = result.emplace();
        resultSeq.idxs.reserve(matches.count());

        for (int idx : seq.idxs)
        {
            for (int groupIdx = 0; groupIdx != mergeMotions_.count(); ++groupIdx)
                if (mergeMotions_[groupIdx].findFirst(states[idx].motion) != mergeMotions_[groupIdx].end())
                {
                    // This motion can be merged with any of the other motions in this group.
                    // Add all motions in the group to a map so they map to the same index.
                    // This is used in a second pass to normalize all motion values in the
                    // same group.
                    for (int g = 0; g != mergeMotions_[groupIdx].count(); ++g)
                        groupIdxMap.insertIfNew(mergeMotions_[groupIdx][g], idx);
                    break;
                }
            resultSeq.idxs.push(idx);
        }
    }

    for (auto& seq : result)
        for (int& idx : seq.idxs)
        {
            auto groupIdxIt = groupIdxMap.find(states[idx].motion);
            if (groupIdxIt == groupIdxMap.end())
                continue;
            idx = groupIdxIt->value();
        }

    return result;
}

// ----------------------------------------------------------------------------
void Query::exportDOT(const char* filename, const rfcommon::MotionLabels* labels, rfcommon::FighterID fighterID)
{
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "digraph query {\n");

    auto toHash40OrHex = [labels](rfcommon::FighterMotion motion) -> rfcommon::String {
        if (const char* h40 = labels->lookupHash40(motion))
            return h40;
        return motion.toHex();
    };

    for (int i = 0; i != matchers_.count(); ++i)
    {
        rfcommon::String label =
                i == 0 ? "start" :
                matchers_[i].isWildcard() ? "." :
                toHash40OrHex(matchers_[i].motion_);
        const char* color = matchers_[i].isAcceptCondition() ? "red" : "black";
        fprintf(fp, "m%d [shape=\"record\",color=\"%s\",label=\"%s", i, color, label.cStr());

        if (matchers_[i].inContext(Matcher::HIT))
            fprintf(fp, " | HIT");
        if (matchers_[i].inContext(Matcher::WHIFF))
            fprintf(fp, " | WHIFF");
        if (matchers_[i].inContext(Matcher::SHIELD))
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
