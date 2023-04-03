#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/MotionLabels.hpp"

// ----------------------------------------------------------------------------
States::States(rfcommon::FighterID fighterID, const rfcommon::String& playerName, const rfcommon::String& fighterName)
    : playerName(playerName)
    , fighterName(fighterName)
    , fighterID(fighterID)
{}
States::~States() {}

// ----------------------------------------------------------------------------
Range::Range(int startIdx, int endIdx)
    : startIdx(startIdx), endIdx(endIdx)
{}
Range::~Range() {}

// ----------------------------------------------------------------------------
Sequence::Sequence() {}
Sequence::~Sequence() {}

// ----------------------------------------------------------------------------
rfcommon::String toNotationFallback(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion, const rfcommon::MotionLabels* labels)
{
    if (const char* l = labels->toPreferredNotation(fighterID, motion))
        return l;
    if (const char* h40 = labels->toHash40(motion))
        return h40;
    return motion.toHex();
}

// ----------------------------------------------------------------------------
rfcommon::String toString(const States& states, const Range& range, const rfcommon::MotionLabels* labels)
{
    rfcommon::String result;
    for (int stateIdx = range.startIdx; stateIdx != range.endIdx; ++stateIdx)
    {
        rfcommon::String label;
        const State& state = states[stateIdx];

        if (state.inHitlag() || state.inHitstun())
            label = "disadv";
        else
            label = toNotationFallback(states.fighterID, state.motion, labels);

        if (state.opponentInShieldlag())
            label += " os";
        else if (state.opponentInHitlag() || state.opponentInHitstun())
            label += " adv";

        if (result.length())
            result += " -> ";
        result += label;
    }
    return result;
}

// ----------------------------------------------------------------------------
rfcommon::String toString(const States& states, const Sequence& seq, const rfcommon::MotionLabels* labels)
{
    rfcommon::String result;
    for (int stateIdx : seq.idxs)
    {
        rfcommon::String label;
        const State& state = states[stateIdx];

        if (state.inHitlag() || state.inHitstun())
            label = "disadv";
        else
            label = toNotationFallback(states.fighterID, state.motion, labels);

        if (state.opponentInShieldlag())
            label += " os";
        else if (state.opponentInHitlag() || state.opponentInHitstun())
            label += " adv";

        if (result.length())
            result += " -> ";
        result += label;
    }
    return result;
}
