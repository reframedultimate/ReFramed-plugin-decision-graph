#pragma once

#include "decision-graph/models/DecisionGraph.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {
    class Frame;
}

class GraphBuilderListener;

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
        rfcommon::HashMap<Edge, int, Edge::Hasher> edgeLookup;
        int prevNodeIdx = -1;
    };

    rfcommon::SmallVector<GraphData, 2> graphData_;
    int frameCounter_ = 0;
};
