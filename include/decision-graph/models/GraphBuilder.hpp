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
    void clear();
    void addFrame(const rfcommon::Frame& frame);
    void notifyNewStatsAvailable();

    int numNodes() const;
    int numEdges() const;
    int numFrames() const;

    rfcommon::ListenerDispatcher<GraphBuilderListener> dispatcher;

private:
    rfcommon::SmallVector<DecisionGraph, 2> graphs_;
    int frameCounter_ = 0;
};
