#pragma once

#include "decision-graph/models/State.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"

class SequenceRange;
class LabelMapper;

class States : public rfcommon::Vector<State>
{
public:
    States(rfcommon::FighterID fighterID);
    ~States();

    const rfcommon::FighterID fighterID;
};

class Sequence
{
public:
    struct ConstIterator
    {
        ConstIterator(const Sequence& seq, int idx)
            : seq_(seq)
            , idx_(idx)
        {}

        ConstIterator& operator++()
        {
            idx_++;
            return *this;
        }

        ConstIterator operator++(int)
        {
            ConstIterator tmp(*this);
            operator++();
            return tmp;
        }

        const int operator*() const { return seq_.isRange_ ? idx_ : seq_.idxs_[idx_]; }
        const int operator->() const { return seq_.isRange_ ? idx_ : seq_.idxs_[idx_]; }

        inline bool operator==(const ConstIterator& rhs) { return idx_ == rhs.idx_; }
        inline bool operator!=(const ConstIterator& rhs) { return idx_ != rhs.idx_; }

    private:
        const Sequence& seq_;
        int idx_;
    };

    Sequence();
    Sequence(int startIdx, int endIdx);
    Sequence(rfcommon::Vector<int>&& idxs);
    Sequence(const Sequence& other) = delete;
    Sequence& operator=(const Sequence& other) = delete;
    Sequence(Sequence&& other) noexcept
        : isRange_(other.isRange_)
    {
        if (other.isRange_)
            range_ = other.range_;
        else
            idxs_ = std::move(other.idxs_);
    }
    Sequence& operator=(Sequence&& other) noexcept
    {
        isRange_ = other.isRange_;
        if (other.isRange_)
            range_ = other.range_;
        else
            idxs_ = std::move(other.idxs_);
    }
    ~Sequence();

    const ConstIterator begin() const { return ConstIterator(*this, isRange_ ? range_.startIdx : 0); }
    const ConstIterator end()   const { return ConstIterator(*this, isRange_ ? range_.endIdx : idxs_.count()); }
    int first() const { return isRange_ ? range_.startIdx : idxs_.front(); }
    int last()  const { return isRange_ ? range_.endIdx : idxs_.back(); }
    int count() const { return isRange_ ? 2 : idxs_.count(); }
    int idxAt(int i) { assert(i < count()); return isRange_ ? (i == 0 ? range_.startIdx : range_.endIdx) : idxs_[i]; }

private:
    union {
        rfcommon::Vector<int> idxs_;  // Indices into the state vector
        struct { int startIdx, endIdx; } range_;
    };
    bool isRange_;
};

rfcommon::String toString(const States& states, const Sequence& seq, LabelMapper* labels);
