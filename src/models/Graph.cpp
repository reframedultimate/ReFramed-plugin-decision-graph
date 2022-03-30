#include "decision-graph/models/Graph.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Session.hpp"
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
void Graph::exportDOT(const char* fileName, const rfcommon::Session* session) const
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
        const auto& statusMapping = session->mappingInfo().fighterStatus;
        const rfcommon::String* name = statusMapping.statusToBaseEnumName(nodes[nodeIdx].state.status());
        if (name == nullptr)
            name = statusMapping.statusToFighterSpecificEnumName(nodes[nodeIdx].state.status(), session->fighterID(0));
        fprintf(fp, "  n%d [shape=record,color=\"%f 1.0 1.0\",label=\"{ %s | %" PRIu64 " }\"];\n",
                nodeIdx,
                hue(accIncomingWeights(nodeIdx)),
                name ? name->cStr() : "(unknown)",
                nodes[nodeIdx].state.motion().value());
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
void Graph::exportOGDFSVG(const char* fileName, const rfcommon::Session* session) const
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
        const rfcommon::String defaultName = "(unknown)";
        const auto& statusMapping = session->mappingInfo().fighterStatus;
        const rfcommon::String* name = statusMapping.statusToBaseEnumName(node.state.status());
        if (name == nullptr)
            name = statusMapping.statusToFighterSpecificEnumName(node.state.status(), session->fighterID(0));
        if (name == nullptr)
            name = &defaultName;

        ogdf::node N = G.newNode();
        GA.label(N) = name->cStr();
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
