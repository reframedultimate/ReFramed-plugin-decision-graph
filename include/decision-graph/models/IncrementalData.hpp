#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/Graph.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {
    class Frame;
}

class IncrementalDataListener;

// When looking for existing connections in the graph, we do
// not care about the weight or any other edge attribute
struct EdgeConnectionHasher {
    typedef rfcommon::HashMapHasher<Edge>::HashType HashType;
    HashType operator()(const Edge& edge) const {
        const uint32_t data[2] = {
            static_cast<uint32_t>(edge.from()),
            static_cast<uint32_t>(edge.to())
        };

        return rfcommon::hash32_jenkins_oaat(&data, 8);
    }
};

// Only hashes the node state, not the outgoing/incoming connections,
// since we only care about unique states when building the graph
struct NodeStateHasher {
    typedef rfcommon::HashMapHasher<Node>::HashType HashType;
    HashType operator()(const Node& node) const {
        return State::Hasher()(node.state);
    }
};

class IncrementalData
{
public:
    void prepareNew(int fighterCount);
    void addFrame(const rfcommon::Frame& frame);
    void notifyNewStatsAvailable();

    int numFrames() const;

    void buildExample1();

    const Sequence& sequence(int fighterIdx) const
        { return sequences_[fighterIdx]; }

    const Graph& graph(int fighterIdx) const
        { return graphData_[fighterIdx].graph; }

    rfcommon::ListenerDispatcher<IncrementalDataListener> dispatcher;

private:
    struct GraphData
    {
        Graph graph;
        rfcommon::HashMap<Node, int, NodeStateHasher> nodeLookup;
        rfcommon::HashMap<Edge, int, EdgeConnectionHasher> edgeLookup;
    };

    rfcommon::SmallVector<Sequence, 2> sequences_;
    rfcommon::SmallVector<GraphData, 2> graphData_;
    int frameCounter_ = 0;
};
