#include "decision-graph/models/GraphBuilder.hpp"
#include "decision-graph/listeners/GraphBuilderListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/hash40.hpp"

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

// ----------------------------------------------------------------------------
void GraphBuilder::buildExample1()
{
    Node nair(rfcommon::hash40("attack_air_n"), 0, 0, false, false, false, false);
    Node utilt(rfcommon::hash40("attack_hi_3"), 0, 0, false, false, false, false);
    Node c(rfcommon::hash40("c"), 0, 0, false, false, false, false);
    Node d(rfcommon::hash40("c"), 0, 0, false, false, false, false);
    Node e(rfcommon::hash40("c"), 0, 0, false, false, false, false);
    Node f(rfcommon::hash40("c"), 0, 0, false, false, false, false);

    GraphData& data = graphData_[0];
    data.graph.nodes.clear();
    data.graph.edges.clear();

    data.graph.nodes.push(nair);
    data.graph.nodes.push(utilt);
    data.graph.nodes.push(c);
    data.graph.nodes.push(d);
    data.graph.nodes.push(e);
    data.graph.nodes.push(f);

    data.graph.edges.emplace(2, 0);
    data.graph.edges.emplace(0, 3);
    data.graph.edges.emplace(0, 4);
    data.graph.edges.emplace(3, 2);
    data.graph.edges.emplace(0, 5);
    data.graph.edges.emplace(2, 5);
    data.graph.edges.emplace(5, 1);

    data.graph.nodes[0].outgoingEdges.push(1);
    data.graph.nodes[0].outgoingEdges.push(2);
    data.graph.nodes[0].outgoingEdges.push(4);
    data.graph.nodes[2].outgoingEdges.push(0);
    data.graph.nodes[2].outgoingEdges.push(5);
    data.graph.nodes[3].outgoingEdges.push(3);
    data.graph.nodes[5].outgoingEdges.push(6);
}

