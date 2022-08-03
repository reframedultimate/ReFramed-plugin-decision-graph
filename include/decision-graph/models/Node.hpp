#pragma once

#include "decision-graph/models/State.hpp"

class Node
{
public:
    Node(const State& state) : state(state) {}

    State state;
    rfcommon::Vector<int> outgoingEdges;
    rfcommon::Vector<int> incomingEdges;
};

// Only hashes the node state, not the outgoing/incoming connections,
// since we only care about unique states when building the graph
struct NodeStateHasher {
    typedef State::Hasher::HashType HashType;
    HashType operator()(const Node& node) const {
        return State::Hasher()(node.state);
    }
};
