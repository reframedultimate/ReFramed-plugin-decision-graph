#pragma once

#include "decision-graph/models/State.hpp"
#include "rfcommon/Vector.hpp"

class Sequence
{
public:
    rfcommon::Vector<State> states;
};
