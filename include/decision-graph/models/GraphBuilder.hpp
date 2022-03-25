#pragma once

#include "decision-graph/models/DecisionGraph.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {
    class Frame;
}

class GraphBuilderListener;

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

class GraphBuilder
{
public:
    void prepareNew(int fighterCount);
    void addFrame(const rfcommon::Frame& frame);
    void notifyNewStatsAvailable();

    int numFrames() const;

    const DecisionGraph& graph(int fighterIdx) const
        { return graphData_[fighterIdx].graph; }

    rfcommon::ListenerDispatcher<GraphBuilderListener> dispatcher;

private:
    struct GraphData
    {
        DecisionGraph graph;
        rfcommon::HashMap<Node, int, Node::Hasher> nodeLookup;
        rfcommon::HashMap<Edge, int, EdgeConnectionHasher> edgeLookup;
        int prevNodeIdx = -1;
    };

    rfcommon::SmallVector<GraphData, 2> graphData_;
    int frameCounter_ = 0;
};
