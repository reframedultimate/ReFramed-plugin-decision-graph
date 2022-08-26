#pragma once

#include "decision-graph/models/State.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"
#include <variant>

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
    Range(int startIdx, int endIdx);
    ~Range();

    int startIdx, endIdx;
};

class Sequence
{
public:
    Sequence();
    ~Sequence();

    rfcommon::SmallVector<int, 8> idxs;
};

rfcommon::String toString(const States& states, const Range& range, LabelMapper* labels);
rfcommon::String toString(const States& states, const Sequence& seq, LabelMapper* labels);
