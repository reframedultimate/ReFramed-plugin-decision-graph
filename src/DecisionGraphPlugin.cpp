#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/GraphBuilder.hpp"
#include "rfcommon/SavedGameSession.hpp"

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

    graphBuilder_->notifyNewStatsAvailable();
    graphBuilder_->graph(0).exportDOT("decision_graph.dot", session);
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
}
