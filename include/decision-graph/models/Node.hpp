#pragma once

#include "decision-graph/models/State.hpp"

class Node
{
public:
    State state;
    rfcommon::Vector<int> outgoingEdges;
    rfcommon::Vector<int> incomingEdges;
};
