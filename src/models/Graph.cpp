#include "decision-graph/models/Graph.hpp"
#include "decision-graph/models/LabelMapper.hpp"

#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"

#include <cstdio>
#include <cinttypes>

#include "ogdf/energybased/FMMMLayout.h"
#include "ogdf/fileformats/GraphIO.h"

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
Graph Graph::cutLoopsIncoming() const
{
    rfcommon::SmallVector<int, 256> stack;
    rfcommon::SmallVector<int, 256> parents;
    rfcommon::SmallVector<int, 256> children;
    rfcommon::SmallVector<int, 64> sourceNodes;
    auto visited = rfcommon::SmallVector<char, 256>::makeResized(nodes.count());

    return Graph();

    stack.push(0);
    while (stack.count())
    {
        int node = stack.popValue();

        // If node has no incoming children, it is a source leaf node
        if (nodes[node].incomingEdges.count() == 0)
            sourceNodes.push(node);
        else
        {
            for (int edge : nodes[node].incomingEdges)
                if (visited[edges[edge].from()]++ == 0)
                    stack.push(edges[edge].from());
            for (int edge : nodes[node].outgoingEdges)
                if (visited[edges[edge].to()]++ == 0)
                    stack.push(edges[edge].to());
        }
    }

    Graph result;
    for (int source : sourceNodes)
    {
        result.nodes.push(Node(nodes[source]));
        parents.push(source);
        stack.push(source);

        for (int edge : nodes[source].outgoingEdges)
            if (visited[edges[edge].to()]++ == 0)
                children.push(edges[edge].to());

        memset(visited.data(), 0, visited.count());
        while (children.count())
        {
            int child = children.popValue();
            int parent = parents.popValue();

            result.nodes.emplace(nodes[child].state);
            result.edges.emplace(result.nodes.count() - 2, result.nodes.count() - 1);
            result.nodes.back(2).outgoingEdges.push(result.edges.count() - 1);
            result.nodes.back(1).incomingEdges.push(result.edges.count() - 1);

            stack.push(child);

            for (int edge : nodes[child].outgoingEdges)
                if (visited[edges[edge].to()]++ == 0)
                {
                    parents.push(child);
                    children.push(edges[edge].to());
                }
                else
                {
                    int subWeight = edges[edge].weight();
                    // TODO
                }
        }
    }
}

// ----------------------------------------------------------------------------
Graph Graph::cutLoopsOutgoing() const
{
    return Graph();
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Graph> Graph::islands() const
{
    return rfcommon::Vector<Graph>();
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Graph> Graph::treeIslands() const
{
    return rfcommon::Vector<Graph>();
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Graph::UniqueSequence> Graph::uniqueSinks() const
{
    return rfcommon::Vector<Graph::UniqueSequence>();
}

// ----------------------------------------------------------------------------
rfcommon::Vector<Graph::UniqueSequence> Graph::uniqueSources() const
{
    return rfcommon::Vector<Graph::UniqueSequence>();
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
            labels->bestEffortString(fighterID, nodes[nodeIdx].state.motion()).cStr(),
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
        GA.label(N) = labels->bestEffortString(fighterID, node.state.motion()).cStr();
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
