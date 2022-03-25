#include "decision-graph/models/DecisionGraph.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Session.hpp"
#include <cstdio>

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
        fprintf(fp, "  n%d [label=\"%s\"];\n", nodeIdx, name ? name->cStr() : "(unknown)");
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
