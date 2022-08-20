#include "decision-graph/models/Graph.hpp"
#include "decision-graph/models/LabelMapper.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"

#include <cstdio>
#include <cinttypes>

#include "ogdf/energybased/FMMMLayout.h"
#include "ogdf/fileformats/GraphIO.h"

namespace {

template <int N>
class VisitedBitmap
{
public:
    static VisitedBitmap make(int nodeCount) { return VisitedBitmap(nodeCount); }
    void reset() { memset(vec_.data(), 0, vec_.count()); }

    bool canVisit(int nodeIdx)
    {
        const int idx = nodeIdx / 8;
        const unsigned char bitPos = static_cast<unsigned int>(nodeIdx) & 0x07;
        const unsigned char mask = 1 << bitPos;
        bool notVisited = !(vec_[idx] & mask);
        vec_[idx] |= mask;
        return notVisited;
    }

    void visit(int nodeIdx)
    {
        canVisit(nodeIdx);
    }

    bool visited(int nodeIdx)
    {
        return !canVisit(nodeIdx);
    }

private:
    VisitedBitmap(int nodeCount) : vec_(rfcommon::SmallVector<unsigned char, N/8+1>::makeResized(nodeCount)) {}
    rfcommon::SmallVector<unsigned char, N/8+1> vec_;
};

}

// ----------------------------------------------------------------------------
Graph Graph::fromSequenceRanges(const Sequence& sequence, const rfcommon::Vector<SequenceRange>& ranges)
{
    Graph graph;
    rfcommon::HashMap<State, int, State::Hasher> stateLookup;
    rfcommon::HashMap<EdgeConnection, int, EdgeConnection::Hasher> edgeLookup;

    for (const auto& range : ranges)
    {
        int prevNodeIdx = -1;
        for (int stateIdx = range.startIdx; stateIdx != range.endIdx; ++stateIdx)
        {
            const State& state = sequence.states[stateIdx];
            auto nodeLookupResult = stateLookup.insertOrGet(state, -1);
            if (nodeLookupResult->value() == -1)
            {
                graph.nodes.emplace(state);
                nodeLookupResult->value() = graph.nodes.count() - 1;
            }
            const int currentNodeIdx = nodeLookupResult->value();

            if (prevNodeIdx != -1)
            {
                auto edgeLookupResult = edgeLookup.insertOrGet(EdgeConnection(prevNodeIdx, currentNodeIdx), -1);
                if (edgeLookupResult->value() == -1)
                {
                    graph.edges.emplace(prevNodeIdx, currentNodeIdx);
                    graph.nodes[prevNodeIdx].outgoingEdges.push(graph.edges.count() - 1);
                    graph.nodes[currentNodeIdx].incomingEdges.push(graph.edges.count() - 1);

                    edgeLookupResult->value() = graph.edges.count() - 1;
                }
                else
                    graph.edges[edgeLookupResult->value()].addWeight();
            }

            prevNodeIdx = currentNodeIdx;
        }
    }

    return graph;
}

// ----------------------------------------------------------------------------
int Graph::findHighestThroughputNode() const
{
    int highestSeen = 0;
    int idx = 0;
    for (int node = 0; node != nodes.count(); ++node)
    {
        int outgoing = 0;
        int incoming = 0;
        for (int edge : nodes[node].outgoingEdges)
            outgoing += edges[edge].weight();
        for (int edge : nodes[node].incomingEdges)
            incoming += edges[edge].weight();

        int throughput = outgoing < incoming ? outgoing : incoming;
        if (highestSeen < throughput)
        {
            highestSeen = throughput;
            idx = node;
        }
    }

    return idx;
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Graph> Graph::islands() const
{
    rfcommon::Vector<Graph> islands;
    rfcommon::SmallVector<int, 256> children;
    rfcommon::SmallVector<int, 256> parents;
    rfcommon::SmallVector<int, 256> pushedParents;
    rfcommon::SmallLinearMap<int, int, 256> map;
    auto visited = VisitedBitmap<256>::make(nodes.count());

    for (int root = 0; root != nodes.count(); ++root)
    {
        if (visited.visited(root))
            continue;

        Graph graph;
        graph.nodes.emplace(nodes[root].state);
        map.insertNew(root, 0);
        visited.visit(root);

        for (int edge : nodes[root].outgoingEdges)
            if (visited.canVisit(edges[edge].to()))
            {
                parents.push(root);
                children.push(edges[edge].to());
                pushedParents.push(0);
            }
        for (int edge : nodes[root].incomingEdges)
            if (visited.canVisit(edges[edge].from()))
            {
                parents.push(root);
                children.push(edges[edge].from());
                pushedParents.push(0);
            }

        while (children.count())
        {
            int child = children.popValue();
            int parent = parents.popValue();
            int pushedParent = pushedParents.popValue();

            // Insert new node (we call it the "pushed child")
            int pushedChild = graph.nodes.count();
            graph.nodes.emplace(nodes[child].state);
            map.insertNew(child, pushedChild);

            // Connect new node to parent. Can be incoming or outgoing connection
            for (int edge : nodes[child].outgoingEdges)
                if (edges[edge].to() == parent)
                {
                    graph.edges.emplace(pushedChild, pushedParent, edges[edge].weight());
                    graph.nodes[pushedChild].outgoingEdges.push(graph.edges.count() - 1);
                    graph.nodes[pushedParent].incomingEdges.push(graph.edges.count() - 1);
                    goto edge_pushed;
                }
            for (int edge : nodes[child].incomingEdges)
                if (edges[edge].from() == parent)
                {
                    graph.edges.emplace(pushedParent, pushedChild, edges[edge].weight());
                    graph.nodes[pushedParent].outgoingEdges.push(graph.edges.count() - 1);
                    graph.nodes[pushedChild].incomingEdges.push(graph.edges.count() - 1);
                    goto edge_pushed;
                }
            assert(false);
            edge_pushed:;

            // Go to next children
            for (int edge : nodes[child].outgoingEdges)
                if (visited.canVisit(edges[edge].to()))
                {
                    parents.push(child);
                    children.push(edges[edge].to());
                    pushedParents.push(pushedChild);
                }
                else  // May have to close loop, since this is a graph
                {
                    auto it = map.findKey(edges[edge].to());
                    if (it == map.end())
                        continue;
                    int pushedVisitedNode = it->value();
                    for (int pushedEdge : graph.nodes[pushedChild].outgoingEdges)
                        if (graph.edges[pushedEdge].to() == pushedVisitedNode)
                            goto outgoing_edge_already_copied;

                    graph.edges.emplace(pushedChild, pushedVisitedNode, edges[edge].weight());
                    graph.nodes[pushedChild].outgoingEdges.push(graph.edges.count() - 1);
                    graph.nodes[pushedVisitedNode].incomingEdges.push(graph.edges.count() - 1);
                    outgoing_edge_already_copied: continue;
                }
            for (int edge : nodes[child].incomingEdges)
                if (visited.canVisit(edges[edge].from()))
                {
                    parents.push(child);
                    children.push(edges[edge].from());
                    pushedParents.push(pushedChild);
                }
                else  // May have to close loop, since this is a graph
                {
                    auto it = map.findKey(edges[edge].from());
                    if (it == map.end())
                        continue;
                    int pushedVisitedNode = it->value();
                    for (int pushedEdge : graph.nodes[pushedChild].incomingEdges)
                        if (graph.edges[pushedEdge].from() == pushedVisitedNode)
                            goto incoming_edge_already_copied;

                    graph.edges.emplace(pushedVisitedNode, pushedChild, edges[edge].weight());
                    graph.nodes[pushedVisitedNode].outgoingEdges.push(graph.edges.count() - 1);
                    graph.nodes[pushedChild].incomingEdges.push(graph.edges.count() - 1);
                    incoming_edge_already_copied: continue;
                }
        }

        islands.push(std::move(graph));
    }

    return islands;
}

// ----------------------------------------------------------------------------
Graph Graph::outgoingTree() const
{
    rfcommon::SmallVector<int, 256> children;
    rfcommon::SmallVector<int, 256> parents;
    rfcommon::SmallVector<int, 256> pushedParents;
    auto visited = VisitedBitmap<256>::make(nodes.count());

    Graph result;

    int root = findHighestThroughputNode();
    {
        int idx = -1;
        int highestSeen = 0;
        for (int edge : nodes[root].incomingEdges)
            if (highestSeen < edges[edge].weight() && nodes[root].state.status() == nodes[edges[edge].from()].state.status())
            {
                highestSeen = edges[edge].weight();
                idx = edge;
            }
        if (idx != -1)
            root = edges[idx].from();
    }

    result.nodes.emplace(nodes[root].state);
    visited.visit(root);

    for (int edge : nodes[root].outgoingEdges)
        if (visited.canVisit(edges[edge].to()))
        {
            parents.push(root);
            children.push(edges[edge].to());
            pushedParents.push(0);
        }

    while (children.count())
    {
        int child = children.popValue();
        int parent = parents.popValue();
        int pushedParent = pushedParents.popValue();

        int pushedChild = result.nodes.count();
        result.nodes.emplace(nodes[child].state);
        for (int edge : nodes[child].incomingEdges)
            if (edges[edge].from() == parent)
            {
                result.edges.emplace(pushedParent, pushedChild, edges[edge].weight());
                result.nodes[pushedParent].outgoingEdges.push(result.edges.count() - 1);
                result.nodes[pushedChild].incomingEdges.push(result.edges.count() - 1);
                goto edge_pushed;
            }
        assert(false);
        edge_pushed:;

        for (int edge : nodes[child].outgoingEdges)
            if (visited.canVisit(edges[edge].to()))
            {
                parents.push(child);
                children.push(edges[edge].to());
                pushedParents.push(pushedChild);
            }
    }

    return result;
}

// ----------------------------------------------------------------------------
Graph Graph::incomingTree() const
{
    rfcommon::SmallVector<int, 256> children;
    rfcommon::SmallVector<int, 256> parents;
    rfcommon::SmallVector<int, 256> pushedParents;
    auto visited = VisitedBitmap<256>::make(nodes.count());

    Graph result;

    int root = findHighestThroughputNode();
    {
        int idx = -1;
        int highestSeen = 0;
        for (int edge : nodes[root].outgoingEdges)
            if (highestSeen < edges[edge].weight() && nodes[root].state.status() == nodes[edges[edge].to()].state.status())
            {
                highestSeen = edges[edge].weight();
                idx = edge;
            }
        if (idx != -1)
            root = edges[idx].to();
    }

    result.nodes.emplace(nodes[root].state);
    visited.visit(root);

    for (int edge : nodes[root].incomingEdges)
        if (visited.canVisit(edges[edge].from()))
        {
            parents.push(root);
            children.push(edges[edge].from());
            pushedParents.push(0);
        }

    while (children.count())
    {
        int child = children.popValue();
        int parent = parents.popValue();
        int pushedParent = pushedParents.popValue();

        int pushedChild = result.nodes.count();
        result.nodes.emplace(nodes[child].state);
        for (int edge : nodes[child].outgoingEdges)
            if (edges[edge].to() == parent)
            {
                result.edges.emplace(pushedChild, pushedParent, edges[edge].weight());
                result.nodes[pushedChild].outgoingEdges.push(result.edges.count() - 1);
                result.nodes[pushedParent].incomingEdges.push(result.edges.count() - 1);
                goto edge_pushed;
            }
        assert(false);
        edge_pushed:;

        for (int edge : nodes[child].incomingEdges)
            if (visited.canVisit(edges[edge].from()))
            {
                parents.push(child);
                children.push(edges[edge].from());
                pushedParents.push(pushedChild);
            }
    }

    return result;
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Graph::UniqueSequence> Graph::treeToUniuqeOutgoingSequences() const
{
    rfcommon::SmallVector<int, 256> stack;
    rfcommon::SmallVector<int, 256> leafNodes;
    rfcommon::Vector<UniqueSequence> result;

    stack.push(0);
    while (stack.count())
    {
        int node = stack.popValue();

        if (nodes[node].outgoingEdges.count() == 0)
            leafNodes.push(node);

        for (int edge : nodes[node].outgoingEdges)
            stack.push(edges[edge].to());
    }

    while (leafNodes.count())
    {
        int node = leafNodes.popValue();
        int weight = nodes[node].incomingEdges.count() ? edges[nodes[node].incomingEdges[0]].weight() : 1;

        Sequence seq;
        while (1)
        {
            seq.states.insert(0, nodes[node].state);
            assert(nodes[node].incomingEdges.count() <= 1);
            if (nodes[node].incomingEdges.count() == 0)
                break;
            node = edges[nodes[node].incomingEdges[0]].from();
        }

        result.push({ seq, weight });
    }

    return result;
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Graph::UniqueSequence> Graph::treeToUniqueIncomingSequences() const
{
    rfcommon::SmallVector<int, 256> stack;
    rfcommon::SmallVector<int, 256> leafNodes;
    rfcommon::Vector<UniqueSequence> result;

    stack.push(0);
    while (stack.count())
    {
        int node = stack.popValue();

        if (nodes[node].incomingEdges.count() == 0)
            leafNodes.push(node);

        for (int edge : nodes[node].incomingEdges)
            stack.push(edges[edge].from());
    }

    while (leafNodes.count())
    {
        int node = leafNodes.popValue();
        int weight = nodes[node].outgoingEdges.count() ? edges[nodes[node].outgoingEdges[0]].weight() : 1;

        Sequence seq;
        while (1)
        {
            seq.states.push(nodes[node].state);
            assert(nodes[node].outgoingEdges.count() <= 1);
            if (nodes[node].outgoingEdges.count() == 0)
                break;
            node = edges[nodes[node].outgoingEdges[0]].to();
        }

        result.push({ seq, weight });
    }

    return result;
}

// ----------------------------------------------------------------------------
void Graph::exportDOT(const char* fileName, rfcommon::FighterID fighterID, const LabelMapper* labels) const
{
    FILE* fp = fopen(fileName, "wb");
    if (fp == nullptr)
        return;

    const float maxWeight = [this]() -> float {
        float weight = 1.0;
        for (const auto& edge : edges)
            if (weight < edge.weight())
                weight = edge.weight();
        return weight;
    }();

    auto accIncomingWeights = [this](int nodeIdx) -> float {
        float weight = 0.0;
        for (const auto& edgeIdx : nodes[nodeIdx].incomingEdges)
            weight += edges[edgeIdx].weight();
        return weight;
    };

    auto hue = [&maxWeight](float weight) -> float {
        weight = (weight - 1.0) / (maxWeight - 1.0);  // Scale to [0..1]
        weight = std::pow(weight, 0.1);  // better distribution
        return (2.0 / 3.0) - (weight * 2.0 / 3.0);  // hue goes from 0 (red) to 0.666 (blue)
    };

    fprintf(fp, "digraph decisions {\n");

    for (int nodeIdx = 0; nodeIdx != nodes.count(); ++nodeIdx)
    {
        rfcommon::String flags;
        if (nodes[nodeIdx].state.inHitlag())
            flags += rfcommon::String("| hitlag");
        if (nodes[nodeIdx].state.inHitstun())
            flags += rfcommon::String(flags.count() ? ", hitstun" : "| hitstun");
        if (nodes[nodeIdx].state.inShieldlag())
            flags += rfcommon::String(flags.count() ? ", shieldlag" : "| shieldlag");
        if (nodes[nodeIdx].state.opponentInHitlag())
            flags += rfcommon::String(flags.count() ? ", op hitlag" : "| op hitlag");
        if (nodes[nodeIdx].state.opponentInHitstun())
            flags += rfcommon::String(flags.count() ? ", op hitstun" : "| op hitstun");
        if (nodes[nodeIdx].state.opponentInShieldlag())
            flags += rfcommon::String(flags.count() ? ", op shieldlag" : "| op shieldlag");

        fprintf(fp, "  n%d [shape=record,color=\"%f 1.0 1.0\",label=\"{ %s %s }\"];\n",
            nodeIdx,
            hue(accIncomingWeights(nodeIdx)),
            labels->bestEffortStringAllLayers(fighterID, nodes[nodeIdx].state.motion()).cStr(),
            flags.cStr());
    }

    for (int edgeIdx = 0; edgeIdx != edges.count(); ++edgeIdx)
    {
        fprintf(fp, "  n%d -> n%d [label=\"%d\", weight=%d];\n",
                edges[edgeIdx].from(),
                edges[edgeIdx].to(),
                edges[edgeIdx].weight(),
                edges[edgeIdx].weight());
    }

    fprintf(fp, "}\n");
    fclose(fp);
}

// ----------------------------------------------------------------------------
void Graph::exportOGDFSVG(const char* fileName, rfcommon::FighterID fighterID, const LabelMapper* labels) const
{
    ogdf::Graph G;
    ogdf::GraphAttributes GA(G,
        ogdf::GraphAttributes::nodeGraphics
      | ogdf::GraphAttributes::edgeGraphics
      | ogdf::GraphAttributes::nodeLabel
      | ogdf::GraphAttributes::edgeStyle
      | ogdf::GraphAttributes::nodeStyle
      | ogdf::GraphAttributes::nodeTemplate);

    rfcommon::Vector<ogdf::node> ogdfNodes;

    for (const auto& node : nodes)
    {
        ogdf::node N = G.newNode();
        GA.label(N) = labels->bestEffortStringAllLayers(fighterID, node.state.motion()).cStr();
        GA.width(N) = 250;
        GA.height(N) = 20;
        ogdfNodes.push(N);
    }

    for (int edgeIdx = 0; edgeIdx != edges.count(); ++edgeIdx)
    {
        const int from = edges[edgeIdx].from();
        const int to = edges[edgeIdx].to();
        G.newEdge(ogdfNodes[from], ogdfNodes[to]);
    }

    ogdf::FMMMLayout fmmm;
    fmmm.useHighLevelOptions(true);
    fmmm.unitEdgeLength(15.0);
    fmmm.newInitialPlacement(true);
    fmmm.qualityVersusSpeed(ogdf::FMMMOptions::QualityVsSpeed::GorgeousAndEfficient);
    fmmm.call(GA);
    ogdf::GraphIO::write(GA, fileName);
}
