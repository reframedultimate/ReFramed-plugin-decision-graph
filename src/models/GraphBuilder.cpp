#include "decision-graph/models/GraphBuilder.hpp"
#include "decision-graph/listeners/GraphBuilderListener.hpp"
#include "rfcommon/Frame.hpp"

// ----------------------------------------------------------------------------
void GraphBuilder::clear()
{
    graphs_.clearCompact();
    frameCounter_ = 0;
}

// ----------------------------------------------------------------------------
void GraphBuilder::addFrame(const rfcommon::Frame& frame)
{
    if (graphs_.count() < frame.fighterCount())
        graphs_.resize(frame.fighterCount());

    graphs_[0].addState(0, frame);
    frameCounter_++;
}

// ----------------------------------------------------------------------------
void GraphBuilder::notifyNewStatsAvailable()
{
    dispatcher.dispatch(&GraphBuilderListener::onGraphBuilderNewStats);
}

// ----------------------------------------------------------------------------
int GraphBuilder::numNodes() const
{
    return graphs_[0].numNodes();
}

// ----------------------------------------------------------------------------
int GraphBuilder::numEdges() const
{
    return graphs_[0].numEdges();
}

// ----------------------------------------------------------------------------
int GraphBuilder::numFrames() const
{
    return frameCounter_;
}
