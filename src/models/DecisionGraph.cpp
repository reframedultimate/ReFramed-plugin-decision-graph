#include "decision-graph/models/DecisionGraph.hpp"
#include "rfcommon/Frame.hpp"
#include <cassert>

// ----------------------------------------------------------------------------
void DecisionGraph::clear()
{
    nodeLookup_.clear();
    nodes_.clearCompact();
    prevNodeIdx_ = -1;
    edges_.clearCompact();
}

// ----------------------------------------------------------------------------
void DecisionGraph::addState(int fighterIdx, const rfcommon::Frame& frame)
{
    const auto& state = frame.fighter(fighterIdx);

    Node node(
        state.motion(),
        state.status(),
        state.hitStatus(),
        true, true, true, true  // TODO
    );

    auto nodeLookupResult = nodeLookup_.insertOrGet(NodeHash(node), -1);

    // New unique state
    if (nodeLookupResult->value() == -1)
    {
        nodes_.push(node);
        nodeLookupResult->value() = nodes_.count() - 1;

        const int nodeIdx = nodeLookupResult->value();
        if (prevNodeIdx_ != -1)
            addEdge(prevNodeIdx_, nodeIdx);
        prevNodeIdx_ = nodeIdx;
    }
}

// ----------------------------------------------------------------------------
void DecisionGraph::addEdge(int from, int to)
{
    edges_.emplace(from, to);
}
