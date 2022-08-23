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
Sequence Sequence::fromRange(int startIdx, int endIdx)
{
    return Sequence(startIdx, endIdx);
}

// ----------------------------------------------------------------------------
Sequence Sequence::fromIndexList(rfcommon::Vector<int>&& idxs)
{
    return Sequence(std::move(idxs));
}

// ----------------------------------------------------------------------------
void Sequence::addIndex(int idx)
{
    assert(isRange_);
    idxs_.push(idx);
}

// ----------------------------------------------------------------------------
rfcommon::String Sequence::toString(rfcommon::FighterID fighterID, LabelMapper* labels) const
{
    rfcommon::String result;
    for (const auto& state : states)
    {
        rfcommon::String label;

        if (state.inHitlag() || state.inHitstun())
            label = "disadv";
        else
            label = labels->bestEffortStringHighestLayer(fighterID, state.motion());

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
