#pragma once

#include "decision-graph/models/State.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"

class SequenceRange;
class LabelMapper;

class Sequence
{
public:
    rfcommon::Vector<State> states;

    rfcommon::String toString(rfcommon::FighterID fighterID, LabelMapper* labels) const;
};

class SequenceRange
{
public:
    SequenceRange(const Sequence& seq) : startIdx(0), endIdx(seq.states.count()) {}
    SequenceRange(int startIdx, int endIdx) : startIdx(startIdx), endIdx(endIdx) {}

    int startIdx;
    int endIdx;
};
