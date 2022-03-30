#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/IncrementalData.hpp"
#include "rfcommon/SavedGameSession.hpp"

#include "decision-graph/models/Query.hpp"

// ----------------------------------------------------------------------------
DecisionGraphPlugin::DecisionGraphPlugin(RFPluginFactory* factory)
    : RealtimePlugin(factory)
    , incData_(new IncrementalData)
    , motionsTable_(MotionsTable::load())
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
    return new SequenceSearchView(incData_.get());
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
    incData_->prepareNew(session->fighterCount());

    for (int frameIdx = 0; frameIdx != session->frameCount(); ++frameIdx)
        incData_->addFrame(session->frame(frameIdx));

    //graphBuilder_->buildExample1();
    incData_->notifyNewStatsAvailable();

    incData_->graph(0).exportOGDFSVG("decision_graph.svg", session);
    incData_->graph(0).exportDOT("decision_graph.dot", session);

    Query query = Query::nair_wildcard_example();
    rfcommon::Vector<SequenceRange> queryResult = query.apply(incData_->sequence(0));
    Graph graph = Graph::fromSequenceRanges(incData_->sequence(0), queryResult);
    graph.exportOGDFSVG("decision_graph_search.svg", session);
    graph.exportDOT("decision_graph_search.dot", session);
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
}
