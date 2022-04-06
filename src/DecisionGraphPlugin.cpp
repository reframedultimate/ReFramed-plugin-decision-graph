#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/UserLabelsModel.hpp"
#include "rfcommon/SavedGameSession.hpp"

#include "decision-graph/models/Query.hpp"

// ----------------------------------------------------------------------------
DecisionGraphPlugin::DecisionGraphPlugin(RFPluginFactory* factory)
    : RealtimePlugin(factory)
    , graphModel_(new GraphModel)
    , userLabelsModel_(new UserLabelsModel)
    , seqSearchModel_(new SequenceSearchModel(userLabelsModel_.get()))
{
#if defined(_WIN32)
    userLabelsModel_->loadMotionLabels("share\\reframed\\data\\plugin-decision-graph\\ParamLabels.csv");
#else
    userLabelsModel_->loadMotionLabels("share/reframed/data/plugin-decision-graph/ParamLabels.csv");
#endif
}

// ----------------------------------------------------------------------------
DecisionGraphPlugin::~DecisionGraphPlugin()
{
}

// ----------------------------------------------------------------------------
QWidget* DecisionGraphPlugin::createView()
{
    // Create new instance of view. The view registers as a listener to this model
    return new SequenceSearchView(seqSearchModel_.get(), graphModel_.get(), userLabelsModel_.get());
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
