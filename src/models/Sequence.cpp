#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/LabelMapper.hpp"

// ----------------------------------------------------------------------------
States::States(rfcommon::FighterID fighterID)
    : fighterID(fighterID)
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
rfcommon::String toString(const States& states, const Range& range, LabelMapper* labels)
{
    rfcommon::String result;
    for (int stateIdx = range.startIdx; stateIdx != range.endIdx; ++stateIdx)
    {
        rfcommon::String label;
        const State& state = states[stateIdx];

        if (state.inHitlag() || state.inHitstun())
            label = "disadv";
        else
            label = labels->bestEffortStringHighestLayer(states.fighterID, state.motion);

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
rfcommon::String toString(const States& states, const Sequence& seq, LabelMapper* labels)
{
    rfcommon::String result;
    for (int stateIdx : seq.idxs)
    {
        rfcommon::String label;
        const State& state = states[stateIdx];

        if (state.inHitlag() || state.inHitstun())
            label = "disadv";
        else
            label = labels->bestEffortStringHighestLayer(states.fighterID, state.motion);

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
