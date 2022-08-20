#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/LabelMapper.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/Query.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
DecisionGraphPlugin::DecisionGraphPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : Plugin(factory)
    , labelMapper_(new LabelMapper(userLabels, hash40Strings))
    , graphModel_(new GraphModel)
    , seqSearchModel_(new SequenceSearchModel(labelMapper_.get()))
{
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
    return new SequenceSearchView(seqSearchModel_.get(), graphModel_.get(), labelMapper_.get());
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

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onProtocolTrainingStarted(rfcommon::Session* training)
{ 
    if (state_ != TRAINING)
        seqSearchModel_->clearAll();
    state_ = TRAINING;

    seqSearchModel_->startNewSession(training->tryGetMappingInfo(), training->tryGetMetaData());
}
void DecisionGraphPlugin::onProtocolTrainingResumed(rfcommon::Session* training) 
{
    if (state_ != TRAINING)
        seqSearchModel_->clearAll();
    state_ = TRAINING;

    seqSearchModel_->startNewSession(training->tryGetMappingInfo(), training->tryGetMetaData());
    seqSearchModel_->addAllFrames(training->tryGetFrameData());
    seqSearchModel_->applyAllQueries();
}
void DecisionGraphPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    seqSearchModel_->startNewSession(newTraining->tryGetMappingInfo(), newTraining->tryGetMetaData());
}
void DecisionGraphPlugin::onProtocolTrainingEnded(rfcommon::Session* training) {}
void DecisionGraphPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    if (state_ != GAME)
        seqSearchModel_->clearAll();
    state_ = GAME;

    seqSearchModel_->startNewSession(game->tryGetMappingInfo(), game->tryGetMetaData());
}
void DecisionGraphPlugin::onProtocolGameResumed(rfcommon::Session* game) 
{
    if (state_ != GAME)
        seqSearchModel_->clearAll();
    state_ = GAME;

    seqSearchModel_->startNewSession(game->tryGetMappingInfo(), game->tryGetMetaData());
    seqSearchModel_->addAllFrames(game->tryGetFrameData());
    seqSearchModel_->applyAllQueries();
}
void DecisionGraphPlugin::onProtocolGameEnded(rfcommon::Session* game) {}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onGameSessionLoaded(rfcommon::Session* game)
{
    seqSearchModel_->clearAll();
    if (auto map = game->tryGetMappingInfo())
        if (auto mdata = game->tryGetMetaData())
            if (auto fdata = game->tryGetFrameData())
            {
                seqSearchModel_->startNewSession(map, mdata);
                seqSearchModel_->addAllFrames(fdata);
                seqSearchModel_->applyAllQueries();
            }

    state_ = REPLAY;
}
void DecisionGraphPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    seqSearchModel_->clearAll();
    state_ = NONE;
}
void DecisionGraphPlugin::onTrainingSessionLoaded(rfcommon::Session* training) 
{
    seqSearchModel_->clearAll();
    if (auto map = training->tryGetMappingInfo())
        if (auto mdata = training->tryGetMetaData())
            if (auto fdata = training->tryGetFrameData())
            {
                seqSearchModel_->startNewSession(map, mdata);
                seqSearchModel_->applyAllQueries();
                seqSearchModel_->addAllFrames(fdata);
            }

    state_ = REPLAY;
}
void DecisionGraphPlugin::onTrainingSessionUnloaded(rfcommon::Session* training) 
{
    seqSearchModel_->clearAll();
    state_ = NONE;
}
void DecisionGraphPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames) 
{
    seqSearchModel_->clearAll();
    for (int i = 0; i != numGames; ++i)
        if (auto map = games[i]->tryGetMappingInfo())
            if (auto mdata = games[i]->tryGetMetaData())
                if (auto fdata = games[i]->tryGetFrameData())
                {
                    seqSearchModel_->startNewSession(map, mdata);
                    seqSearchModel_->addAllFrames(fdata);
                }
    seqSearchModel_->applyAllQueries();

    state_ = REPLAY;
}
void DecisionGraphPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) 
{
    seqSearchModel_->clearAll();
    state_ = NONE;
}
