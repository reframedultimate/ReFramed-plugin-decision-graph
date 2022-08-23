#pragma once

#include "decision-graph/models/State.hpp"

class Node
{
public:
    Node(int stateIdx) : stateIdx(stateIdx) {}

    int stateIdx;
    rfcommon::Vector<int> outgoingEdges;
    rfcommon::Vector<int> incomingEdges;
};
