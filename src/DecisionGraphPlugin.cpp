#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/GraphBuilder.hpp"
#include "rfcommon/SavedGameSession.hpp"

#include "decision-graph/models/GraphQuery.hpp"

// ----------------------------------------------------------------------------
DecisionGraphPlugin::DecisionGraphPlugin(RFPluginFactory* factory)
    : RealtimePlugin(factory)
    , graphBuilder_(new GraphBuilder)
{
}

// ----------------------------------------------------------------------------
DecisionGraphPlugin::~DecisionGraphPlugin()
{
}

// ----------------------------------------------------------------------------
QWidget* DecisionGraphPlugin::createView()
{
    // Create new instance of view. The view registers as a listener to this model
    return new SequenceSearchView(graphBuilder_.get());
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::destroyView(QWidget* view)
{
    // ReFramed no longer needs the view, delete it
    delete view;
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::setSavedGameSession(rfcommon::SavedGameSession* session)
{
    graphBuilder_->prepareNew(session->fighterCount());

    for (int frameIdx = 0; frameIdx != session->frameCount(); ++frameIdx)
        graphBuilder_->addFrame(session->frame(frameIdx));

    graphBuilder_->buildExample1();
    graphBuilder_->notifyNewStatsAvailable();

    const DecisionGraph& graph = graphBuilder_->graph(0);
    graph.exportOGDFSVG("decision_graph.svg", session);
    graph.exportDOT("decision_graph.dot", session);

    GraphQuery query = GraphQuery::nair_utilt_example();
    DecisionGraph result = query.apply(graph);
    result.exportOGDFSVG("decision_graph_search.svg", session);
    result.exportDOT("decision_graph_search.dot", session);
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
}
