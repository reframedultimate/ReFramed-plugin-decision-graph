#include "decision-graph/models/GraphQuery.hpp"
#include "rfcommon/hash40.hpp"
#include <cstdio>

// ----------------------------------------------------------------------------
NodeMatcher NodeMatcher::wildCard()
{
    return NodeMatcher(
        0,
        0,
        0,
        0
    );
}

// ----------------------------------------------------------------------------
NodeMatcher NodeMatcher::motion(rfcommon::FighterMotion motion)
{
    return NodeMatcher(
        motion,
        0,
        HIT_CONNECT | HIT_WHIFF | HIT_ON_SHIELD,
        MATCH_MOTION
    );
}

// ----------------------------------------------------------------------------
NodeMatcher::NodeMatcher(
        rfcommon::FighterMotion motion,
        rfcommon::FighterStatus status,
        uint8_t hitType,
        uint8_t matchFlags)
    : motion_(motion)
    , status_(status)
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
GraphQuery GraphQuery::nair_mixup_example()
{
    GraphQuery query;
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("attack_air_n")));
    query.matchers_.push(NodeMatcher::wildCard());
    query.matchers_.push(NodeMatcher::wildCard());
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("attack_hi3")));
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("catch")));
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("escape_b")));
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("escape_f")));

    query.matchers_[0].next.push(1);
    query.matchers_[0].next.push(2);
    query.matchers_[1].next.push(2);
    query.matchers_[2].next.push(3);
    query.matchers_[2].next.push(4);
    query.matchers_[2].next.push(5);
    query.matchers_[2].next.push(6);

    return query;
}

// ----------------------------------------------------------------------------
GraphQuery GraphQuery::nair_wildcard_example()
{
    GraphQuery query;
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("attack_air_n")));
    query.matchers_.push(NodeMatcher::motion(rfcommon::hash40("landing_air_n")));
    query.matchers_.push(NodeMatcher::wildCard());
    query.matchers_.push(NodeMatcher::wildCard());

    query.matchers_[0].next.push(1);
    query.matchers_[1].next.push(2);
    //query.matchers_[2].next.push(3);
    //query.matchers_[3].next.push(4);

    return query;
}

// ----------------------------------------------------------------------------
DecisionGraph GraphQuery::apply(const DecisionGraph& graph)
{

    struct InterGraphMapping
    {
        InterGraphMapping(int src, int dst) : srcGraphNodeIdx(src), dstGraphNodeIdx(dst) {}
        int srcGraphNodeIdx;
        int dstGraphNodeIdx;
    };
    rfcommon::HashMap<Node, InterGraphMapping, Node::Hasher> foundNodes;
    DecisionGraph result;

    // Returns the ending index in the sequence (exclusive) for the
    // current search pattern. If no pattern was found then this returns
    // the starting index
    auto doSequenceMatch = [this, &graph](const int seqStartIdx) -> int {
        int currentMatcherIdx = 0;
        int seqIdx = seqStartIdx + 1;
        while (matchers_[currentMatcherIdx].next.count() > 0)
        {

            // State machine is not complete but there are no more nodes to match.
            // Can't find any match, return
            if (seqIdx >= graph.nodeSequence.count())
                return seqStartIdx;

            for (int nextMatcherIdx : matchers_[currentMatcherIdx].next)
            {
                const int nodeIdx = graph.nodeSequence[seqIdx];
                if (matchers_[nextMatcherIdx].matches(graph.nodes[nodeIdx]))
                {
                    currentMatcherIdx = nextMatcherIdx;  // Advance state machine
                    seqIdx++;  // Next node in sequence
                    goto matchFound;
                }
            }
            return seqStartIdx;  // Node did not match search pattern, return

            matchFound:;
        }

        return seqIdx;
    };

    // Nothing to do
    if (matchers_.count() == 0)
        return result;

    // We search the sequence of nodes rather than the graph, because we are
    // interested in matching sequences of decisions
    for (int seqIdx = 0; seqIdx != graph.nodeSequence.count(); ++seqIdx)
    {
        int currentMatcherIdx = 0;
        int nodeIdx = graph.nodeSequence[seqIdx];
        if (matchers_[currentMatcherIdx].matches(graph.nodes[nodeIdx]) == false)
            continue;

        // Found a node that matches the starting condition
        const int seqEndIdx = doSequenceMatch(seqIdx);
        for (int i = seqIdx; i != seqEndIdx; ++i)
        {
            const int nodeIdx = graph.nodeSequence[i];
            foundNodes.insertNew(graph.nodes[nodeIdx], InterGraphMapping(nodeIdx, 0));
        }
    }

    // First, insert all nodes we found into the new result graph, and fill in
    // the indices of each node from the new graph into the foundNodes hash
    // table so we have a mapping between old graph indices and new graph
    // indices
    for (auto kv : foundNodes)
    {
        result.nodes.push(kv.key().copyWithoutEdges());
        kv.value().dstGraphNodeIdx = result.nodes.count() - 1;
    }

    for (const auto& fromkv : foundNodes)
    {
        const Node& from = fromkv.key();
        for (int edgeIdx : fromkv.key().outgoingEdges)
        {
            const Node& to = graph.nodes[graph.edges[edgeIdx].to()];
            const auto tokv = foundNodes.find(to);
            if (tokv == foundNodes.end())
                continue;

            result.edges.emplace(fromkv.value().dstGraphNodeIdx, tokv->value().dstGraphNodeIdx, graph.edges[edgeIdx].weight());
        }
    }

    return result;
}
