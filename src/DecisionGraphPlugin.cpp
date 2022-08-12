#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/UserLabelsModel.hpp"
#include "decision-graph/models/Query.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
DecisionGraphPlugin::DecisionGraphPlugin(RFPluginFactory* factory)
    : Plugin(factory)
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
rfcommon::Plugin::UIInterface* DecisionGraphPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* DecisionGraphPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* DecisionGraphPlugin::replayInterface() { return this; }
rfcommon::Plugin::VisualizerInterface* DecisionGraphPlugin::visualizerInterface() { return nullptr; }
rfcommon::Plugin::VideoPlayerInterface* DecisionGraphPlugin::videoPlayerInterface() { return nullptr; }

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
void DecisionGraphPlugin::onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) {}
void DecisionGraphPlugin::onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) {}
void DecisionGraphPlugin::onProtocolConnectedToServer(const char* ipAddress, uint16_t port) {}
void DecisionGraphPlugin::onProtocolDisconnectedFromServer() {}
void DecisionGraphPlugin::onProtocolTrainingStarted(rfcommon::Session* training) {}
void DecisionGraphPlugin::onProtocolTrainingResumed(rfcommon::Session* training) {}
void DecisionGraphPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) {}
void DecisionGraphPlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}
void DecisionGraphPlugin::onProtocolGameStarted(rfcommon::Session* game) {}
void DecisionGraphPlugin::onProtocolGameResumed(rfcommon::Session* game) {}
void DecisionGraphPlugin::onProtocolGameEnded(rfcommon::Session* game) {}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    seqSearchModel_->setSession(game);
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    seqSearchModel_->clearSession(game);
}
void DecisionGraphPlugin::onTrainingSessionLoaded(rfcommon::Session* training) {}
void DecisionGraphPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) {}
void DecisionGraphPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) {}
void DecisionGraphPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) {}
