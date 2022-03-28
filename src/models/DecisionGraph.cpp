#include "decision-graph/models/DecisionGraph.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Session.hpp"
#include <cstdio>
#include <cinttypes>

#include "ogdf/energybased/FMMMLayout.h"
#include "ogdf/fileformats/GraphIO.h"

// ----------------------------------------------------------------------------
void DecisionGraph::exportDOT(const char* fileName, const rfcommon::Session* session) const
{
    FILE* fp = fopen(fileName, "wb");
    if (fp == nullptr)
        return;

    fprintf(fp, "digraph decisions {\n");

    for (int nodeIdx = 0; nodeIdx != nodes.count(); ++nodeIdx)
    {
        const auto& statusMapping = session->mappingInfo().fighterStatus;
        const rfcommon::String* name = statusMapping.statusToBaseEnumName(nodes[nodeIdx].status());
        if (name == nullptr)
            name = statusMapping.statusToFighterSpecificEnumName(nodes[nodeIdx].status(), session->fighterID(0));
        fprintf(fp, "  n%d [shape=record,label=\"{ %s | %" PRIu64 " }\"];\n", nodeIdx, name ? name->cStr() : "(unknown)", nodes[nodeIdx].motion().value());
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
void DecisionGraph::exportOGDFSVG(const char* fileName, const rfcommon::Session* session) const
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
        const rfcommon::String* name = statusMapping.statusToBaseEnumName(node.status());
        if (name == nullptr)
            name = statusMapping.statusToFighterSpecificEnumName(node.status(), session->fighterID(0));
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
