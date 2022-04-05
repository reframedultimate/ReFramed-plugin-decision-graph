#include "decision-graph/models/GraphModel.hpp"

#include "ogdf/energybased/FMMMLayout.h"
#include "ogdf/fileformats/GraphIO.h"

// ----------------------------------------------------------------------------
void GraphModel::setGraph(const Graph& graph)
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
            name = statusMapping.statusToFighterSpecificEnumName(node.state.status(), session->fighterID(fighterIdx));
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
