#include "decision-graph/models/GraphBuilder.hpp"
#include "decision-graph/listeners/GraphBuilderListener.hpp"
#include "rfcommon/Frame.hpp"

// ----------------------------------------------------------------------------
void GraphBuilder::prepareNew(int fighterCount)
{
    graphData_.clearCompact();
    graphData_.resize(fighterCount);
    frameCounter_ = 0;
}

// ----------------------------------------------------------------------------
void GraphBuilder::addFrame(const rfcommon::Frame& frame)
{
    for (int fighterIdx = 0; fighterIdx != frame.fighterCount(); ++fighterIdx)
    {
        const auto& state = frame.fighter(fighterIdx);
        GraphData& data = graphData_[fighterIdx];

        Node node(
            state.motion(),
            state.status(),
            state.hitStatus(),
            true, true, true, true  // TODO
        );

        // Only process node if it is meaningfully different from the previously
        // processed node
        if (data.prevNodeIdx != -1 && node == data.graph.nodes[data.prevNodeIdx])
            continue;

        // New unique state
        auto nodeLookupResult = data.nodeLookup.insertOrGet(node, -1);
        if (nodeLookupResult->value() == -1)
        {
            data.graph.nodes.push(node);
            nodeLookupResult->value() = data.graph.nodes.count() - 1;
        }
        const int currentNodeIdx = nodeLookupResult->value();

        // Create a new edge to the previously added/visited node
        if (data.prevNodeIdx != -1)
        {
            auto edgeLookupResult = data.edgeLookup.insertOrGet(Edge(data.prevNodeIdx, currentNodeIdx), -1);

            // If edge does not exist yet, create it. Otherwise, add weight to
            // the existing edge
            if (edgeLookupResult->value() == -1)
            {
                data.graph.edges.emplace(data.prevNodeIdx, currentNodeIdx);
                data.graph.nodes[data.prevNodeIdx].outgoingEdges.push(data.graph.edges.count() - 1);
                data.graph.nodes[currentNodeIdx].incomingEdges.push(data.graph.edges.count() - 1);
            }
            else
                data.graph.edges[edgeLookupResult->value()].addWeight();

            edgeLookupResult->value() = data.graph.edges.count() - 1;
        }
        data.prevNodeIdx = currentNodeIdx;
    }

    frameCounter_++;
}

// ----------------------------------------------------------------------------
void GraphBuilder::notifyNewStatsAvailable()
{
    dispatcher.dispatch(&GraphBuilderListener::onGraphBuilderNewStats);
}

// ----------------------------------------------------------------------------
int GraphBuilder::numFrames() const
{
    return frameCounter_;
}
