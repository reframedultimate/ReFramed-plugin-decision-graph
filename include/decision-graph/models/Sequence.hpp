#pragma once

#include "decision-graph/models/State.hpp"
#include "rfcommon/Vector.hpp"

class Sequence
{
public:
    rfcommon::Vector<State> states;
};

class SequenceRange
{
public:
    SequenceRange(int startIdx, int endIdx) : startIdx(startIdx), endIdx(endIdx) {}

    const int startIdx;
    const int endIdx;
};
