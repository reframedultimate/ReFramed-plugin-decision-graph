#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/MotionsTable.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "rfcommon/SavedGameSession.hpp"

#include "decision-graph/models/Query.hpp"

// ----------------------------------------------------------------------------
DecisionGraphPlugin::DecisionGraphPlugin(RFPluginFactory* factory)
    : RealtimePlugin(factory)
    , motionsTable_(MotionsTable::load())
    , seqSearchModel_(new SequenceSearchModel(motionsTable_.get()))
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
    return new SequenceSearchView(seqSearchModel_.get());
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
    seqSearchModel_->setSession(session);
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::clearSavedGameSession(rfcommon::SavedGameSession* session)
{
    seqSearchModel_->clearSession(session);
}
