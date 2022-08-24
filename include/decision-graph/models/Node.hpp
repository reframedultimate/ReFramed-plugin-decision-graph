#pragma once

#include "decision-graph/models/State.hpp"
#include "decision-graph/models/Sequence.hpp"

class Node
{
public:
    Node(int stateIdx) : stateIdx(stateIdx) {}

    int stateIdx;
    rfcommon::Vector<int> outgoingEdges;
    rfcommon::Vector<int> incomingEdges;

    // Only hashes the node state, not the outgoing/incoming connections,
    // since we only care about unique states when building the graph
    /*
    struct StateHasher {
        typedef State::HasherNoSideData::HashType HashType;
        HashType operator()(const Node& node) const {
            State::HasherNoSideData h;
            return h(node.state);
        }
    };*/
};
