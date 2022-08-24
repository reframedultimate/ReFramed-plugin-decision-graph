#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/LabelMapper.hpp"

// ----------------------------------------------------------------------------
States::States(rfcommon::FighterID fighterID)
    : fighterID(fighterID)
{}

// ----------------------------------------------------------------------------
States::~States()
{}

// ----------------------------------------------------------------------------
Sequence::Sequence(int startIdx, int endIdx)
    : range_({startIdx, endIdx})
    , isRange_(true)
{}

// ----------------------------------------------------------------------------
Sequence::Sequence(rfcommon::Vector<int>&& idxs)
    : idxs_(std::move(idxs))
    , isRange_(false)
{}

// ----------------------------------------------------------------------------
Sequence::~Sequence()
{}

// ----------------------------------------------------------------------------
rfcommon::String toString(const States& states, const Sequence& seq, LabelMapper* labels)
{
    rfcommon::String result;
    for (int stateIdx : seq)
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
