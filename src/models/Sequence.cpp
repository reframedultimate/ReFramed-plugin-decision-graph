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
Sequence::Sequence()
    : range_({ 0, 0 })
    , isRange_(true)
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
Sequence::Sequence(const Sequence& other)
    : isRange_(other.isRange_)
{
    if (isRange_)
        range_ = other.range_;
    else
        idxs_ = other.idxs_;
}

// ----------------------------------------------------------------------------
Sequence::Sequence(Sequence&& other) noexcept
    : Sequence()
{
    swap(*this, other);
}

// ----------------------------------------------------------------------------
Sequence::~Sequence()
{}

// ----------------------------------------------------------------------------
Sequence& Sequence::operator=(Sequence other)
{
    swap(*this, other);
    return *this;
}

// ----------------------------------------------------------------------------
void swap(Sequence& first, Sequence& second)
{
    using std::swap;

    if (first.isRange_)
    {
        if (second.isRange_)
            swap(first.range_, second.range_);
        else
        {
            swap(first.isRange_, second.isRange_);

            auto tmpRange = std::move(first.range_);
            first.range_.~Range();

            new (&first.idxs_) rfcommon::Vector<int>(std::move(second.idxs_));
            second.idxs_.~Vector();
            second.range_ = std::move(tmpRange);
        }
    }
    else
    {
        if (second.isRange_)
        {
            swap(first.isRange_, second.isRange_);

            auto tmpRange = std::move(second.range_);
            second.range_.~Range();

            new (&second.idxs_) rfcommon::Vector<int>(std::move(first.idxs_));
            first.idxs_.~Vector();
            first.range_ = std::move(tmpRange);
        }
        else
            swap(first.idxs_, second.idxs_);
    }
}

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
