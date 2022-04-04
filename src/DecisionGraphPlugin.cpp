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
    return new SequenceSearchView(incData_.get(), &motionsTable_);
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
    incData_->setSession(session);

    for (int frameIdx = 0; frameIdx != session->frameCount(); ++frameIdx)
        incData_->addFrame(frameIdx, session->frame(frameIdx));

    incData_->notifyNewStatsAvailable();
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
    incData_->clearSession(session);
}
