#pragma once

#include "decision-graph/models/State.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"
#include <variant>

class SequenceRange;
class LabelMapper;

class States : public rfcommon::Vector<State>
{
public:
    States(rfcommon::FighterID fighterID);
    ~States();

    const rfcommon::FighterID fighterID;
};

class Range
{
public:
    Range(int startIdx, int endIdx) : startIdx(startIdx), endIdx(endIdx) {}
    ~Range() {}

    const int startIdx, endIdx;
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
        const int operator->() const  { return seq_.isRange_ ? idx_ : seq_.idxs_[idx_]; }

        inline bool operator==(const ConstIterator& rhs) { return idx_ == rhs.idx_; }
        inline bool operator!=(const ConstIterator& rhs) { return idx_ != rhs.idx_; }

    private:
        const Sequence& seq_;
        int idx_;
    };

    Sequence();
    Sequence(int startIdx, int endIdx);
    Sequence(rfcommon::SmallVector<int, 2>&& idxs);
    ~Sequence();

    const ConstIterator begin() const { return ConstIterator(*this, isRange_ ? idxs_[0] : 0); }
    const ConstIterator end()   const { return ConstIterator(*this, isRange_ ? idxs_[1] : idxs_.count()); }
    int firstIdx() const { return isRange_ ? idxs_[0] : idxs_.front(); }
    int lastIdx()  const { return isRange_ ? idxs_[1] - 1 : idxs_.back(); }
    int count() const { return isRange_ ? 2 : idxs_.count(); }
    int idxAt(int i) { assert(i < count()); return isRange_ ? (i == 0 ? idxs_[0] : idxs_[1]) : idxs_[i]; }

private:
    rfcommon::SmallVector<int, 2> idxs_;
    bool isRange_;
};

rfcommon::String toString(const States& states, const Sequence& seq, LabelMapper* labels);
