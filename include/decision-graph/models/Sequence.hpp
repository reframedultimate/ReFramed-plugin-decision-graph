#pragma once

#include "decision-graph/models/State.hpp"
#include "rfcommon/Vector.hpp"

class SequenceRange;

class Sequence
{
public:
    rfcommon::Vector<State> states;

    rfcommon::String toString() const;
};

class SequenceRange
{
public:
    SequenceRange(const Sequence& seq) : startIdx(0), endIdx(seq.states.count()) {}
    SequenceRange(int startIdx, int endIdx) : startIdx(startIdx), endIdx(endIdx) {}

    int startIdx;
    int endIdx;
};
