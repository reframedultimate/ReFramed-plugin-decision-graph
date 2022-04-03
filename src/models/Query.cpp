#include "decision-graph/models/Query.hpp"
#include "decision-graph/models/MotionsTable.hpp"
#include "rfcommon/hash40.hpp"
#include <cstdio>
#include <cinttypes>

// ----------------------------------------------------------------------------
Matcher Matcher::start()
{
    return wildCard();
}

// ----------------------------------------------------------------------------
Matcher Matcher::wildCard()
{
    return Matcher(
        0,
        0,
        0,
        0
    );
}

// ----------------------------------------------------------------------------
Matcher Matcher::motion(rfcommon::FighterMotion motion)
{
    return Matcher(
        motion,
        0,
        HIT_CONNECT | HIT_WHIFF | HIT_ON_SHIELD,
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
    , hitType_(hitType)
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

    return true;
}

// ----------------------------------------------------------------------------
Query Query::nair_mixup_example()
{
    Query query;
    query.matchers_.push(Matcher::motion(rfcommon::hash40("attack_air_n")));
    query.matchers_.push(Matcher::wildCard());
    query.matchers_.push(Matcher::wildCard());
    query.matchers_.push(Matcher::motion(rfcommon::hash40("attack_hi3")));
    query.matchers_.push(Matcher::motion(rfcommon::hash40("catch")));
    query.matchers_.push(Matcher::motion(rfcommon::hash40("escape_b")));
    query.matchers_.push(Matcher::motion(rfcommon::hash40("escape_f")));

    query.matchers_[0].next.push(1);
    query.matchers_[0].next.push(2);
    query.matchers_[1].next.push(2);
    query.matchers_[2].next.push(3);
    query.matchers_[2].next.push(4);
    query.matchers_[2].next.push(5);
    query.matchers_[2].next.push(6);

    return query;
}

// ----------------------------------------------------------------------------
Query Query::nair_wildcard_example()
{
    Query query;
    query.matchers_.push(Matcher::wildCard());
    query.matchers_.push(Matcher::motion(rfcommon::hash40("attack_air_n")));
    query.matchers_.push(Matcher::motion(rfcommon::hash40("landing_air_n")));
    query.matchers_.push(Matcher::wildCard());
    query.matchers_.push(Matcher::wildCard());

    query.matchers_[0].next.push(1);
    query.matchers_[1].next.push(2);
    query.matchers_[2].next.push(3);
    //query.matchers_[3].next.push(4);

    return query;
}

// ----------------------------------------------------------------------------
rfcommon::Vector<SequenceRange> Query::apply(const Sequence& seq)
{
    rfcommon::Vector<SequenceRange> result;

    // Returns the ending index in the sequence (exclusive) for the
    // current search pattern. If no pattern was found then this returns
    // the starting index
    auto doSequenceMatch = [this, &seq](const int startIdx) -> int {
        int currentMatcherIdx = 0;
        int stateIdx = startIdx + 1;
        while (matchers_[currentMatcherIdx].next.count() > 0)
        {
            // State machine is not complete but there are no more states to match.
            // Can't find any match, return
            if (stateIdx >= seq.states.count())
                return startIdx;

            for (int nextMatcherIdx : matchers_[currentMatcherIdx].next)
            {
                if (matchers_[nextMatcherIdx].matches(seq.states[stateIdx]))
                {
                    currentMatcherIdx = nextMatcherIdx;  // Advance state machine
                    stateIdx++;  // Next state in sequence
                    goto matchFound;
                }
            }
            return startIdx;  // State did not match search pattern, return

            matchFound:;
        }

        return stateIdx;
    };

    // Nothing to do
    if (matchers_.count() == 0)
        return result;

    // We search the sequence of states rather than the graph, because we are
    // interested in matching sequences of decisions
    for (int stateIdx = 0; stateIdx != seq.states.count(); ++stateIdx)
    {
        int currentMatcherIdx = 0;
        if (matchers_[currentMatcherIdx].matches(seq.states[stateIdx]) == false)
            continue;

        // Found a state that matches the starting condition
        const int stateEndIdx = doSequenceMatch(stateIdx);
        if (stateEndIdx > stateIdx)
            result.emplace(stateIdx, stateEndIdx);
    }

    return result;
}

// ----------------------------------------------------------------------------
void Query::exportDOT(const char* filename, const MotionsTable* table)
{
    FILE* fp = fopen(filename, "w");
    fprintf(fp, "digraph query {\n");

    for (int i = 0; i != matchers_.count(); ++i)
    {
        const char* label = table->motionToLabel(matchers_[i].motion_);
        if (i == 0)
            fprintf(fp, "m%d [label=\"start\"];\n", i);
        else if (label)
            fprintf(fp, "m%d [shape=\"rectangle\",label=\"%s\"];\n", i, label);
        else if (matchers_[i].matchFlags_ == 0)
            fprintf(fp, "m%d [label=\".\"];\n", i);
        else
            fprintf(fp, "m%d [shape=\"rectangle\",label=\"%" PRIu64 "\"];\n",
                    i, matchers_[i].motion_.value());
    }
    for (int i = 0; i != matchers_.count(); ++i)
    {
        for (int e : matchers_[i].next)
            fprintf(fp, "m%d -> m%d;\n", i, e);
    }

    fprintf(fp, "}\n");
    fclose(fp);
}
