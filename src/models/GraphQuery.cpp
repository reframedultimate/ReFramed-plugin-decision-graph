#include "decision-graph/models/GraphQuery.hpp"
#include "rfcommon/hash40.hpp"

// ----------------------------------------------------------------------------
NodeMatcher NodeMatcher::wildCard(int repeats)
{
    return NodeMatcher(
        0,
        0,
        0,
        0,
        repeats
    );
}

// ----------------------------------------------------------------------------
NodeMatcher NodeMatcher::motion(rfcommon::FighterMotion motion)
{
    return NodeMatcher(
        motion,
        0,
        HIT_CONNECT | HIT_WHIFF | HIT_ON_SHIELD,
        MATCH_MOTION,
        1
    );
}

// ----------------------------------------------------------------------------
NodeMatcher::NodeMatcher(
        rfcommon::FighterMotion motion,
        rfcommon::FighterStatus status,
        uint8_t hitType,
        uint8_t matchFlags,
        int repeats)
    : motion_(motion)
    , status_(status)
    , repeats_(repeats)
    , hitType_(hitType)
    , matchFlags_(matchFlags)
{}

// ----------------------------------------------------------------------------
bool NodeMatcher::matches(const Node& node) const
{
    if (!!(matchFlags_ & MATCH_STATUS))
        if (node.status() != status_)
            return false;

    if (!!(matchFlags_ & MATCH_MOTION))
        if (node.motion() != motion_)
            return false;

    return true;
}

// ----------------------------------------------------------------------------
GraphQuery GraphQuery::nair_utilt_example()
{
    GraphQuery query;
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("attack_air_n")));
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("attack_hi_3")));
    query.matchers_.push(NodeMatcher::wildCard());

    query.matchers_[0].next.push(1);
    query.matchers_[0].next.push(2);
    query.matchers_[2].next.push(1);
    query.matchers_[2].next.push(2);

    return query;
}

// ----------------------------------------------------------------------------
bool GraphQuery::applyRecurse(
        const DecisionGraph& graph,
        int nodeIdx,
        int matchIdx,
        rfcommon::Vector<int>* matchedNodes,
        rfcommon::HashMap<Node, int, Node::Hasher>* visited)
{
    if (visited->insertNew(graph.nodes[nodeIdx], nodeIdx) == visited->end())
        return false;

    const auto& matcher = matchers_[matchIdx];
    const auto& node = graph.nodes[nodeIdx];
    if (matcher.matches(node) == false)
        return false;

    for (int edgeIdx : graph.nodes[nodeIdx].outgoingEdges)
        for (matchIdx = 0; matchIdx != matcher.next.count(); ++matchIdx)
            if (applyRecurse(graph, graph.edges[edgeIdx].to(), matchIdx, matchedNodes, visited))
            {
                matchedNodes->push(nodeIdx);
            }

    return false;
}

// ----------------------------------------------------------------------------
DecisionGraph GraphQuery::apply(const DecisionGraph& graph)
{
    DecisionGraph result;

    // Nothing to do
    if (matchers_.count() == 0)
        return result;

    // Find all nodes that match the starting condition
    rfcommon::Vector<int> startingNodes;
    for (int nodeIdx = 0; nodeIdx != graph.nodes.count(); ++nodeIdx)
        if (matchers_[0].matches(graph.nodes[nodeIdx]))
            startingNodes.push(nodeIdx);

    rfcommon::HashMap<Node, int, Node::Hasher> matchingNodes;
    for (int startIdx : startingNodes)
    {
        rfcommon::Vector<int> matchingNodesList;
        rfcommon::HashMap<Node, int, Node::Hasher> visited;
        if (applyRecurse(graph, startIdx, 0, &matchingNodesList, &visited))
        {
            for (int nodeIdx : matchingNodesList)
                matchingNodes.insertNew(graph.nodes[nodeIdx], nodeIdx);
        }
    }

    return result;
}
