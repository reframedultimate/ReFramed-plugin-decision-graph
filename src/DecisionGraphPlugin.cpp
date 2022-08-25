#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/LabelMapper.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/SessionSettingsModel.hpp"
#include "decision-graph/models/Query.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/HighresTimer.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
DecisionGraphPlugin::DecisionGraphPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : Plugin(factory)
    , labelMapper_(new LabelMapper(userLabels, hash40Strings))
    , graphModel_(new GraphModel)
    , seqSearchModel_(new SequenceSearchModel(labelMapper_.get()))
    , sessionSettings_(new SessionSettingsModel)
{
    sessionSettings_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
DecisionGraphPlugin::~DecisionGraphPlugin()
{
    sessionSettings_->dispatcher.removeListener(this);
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
    return new SequenceSearchView(seqSearchModel_.get(), sessionSettings_.get(), graphModel_.get(), labelMapper_.get());
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

    assert(activeSession_.isNull());
    activeSession_ = training;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolTrainingResumed(rfcommon::Session* training) 
{
    if (state_ != TRAINING)
        seqSearchModel_->clearAll();
    state_ = TRAINING;

    seqSearchModel_->startNewSession(training->tryGetMappingInfo(), training->tryGetMetaData());
    seqSearchModel_->addAllFrames(training->tryGetFrameData());
    seqSearchModel_->applyAllQueries();

    assert(activeSession_.isNull());
    activeSession_ = training;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    seqSearchModel_->startNewSession(newTraining->tryGetMappingInfo(), newTraining->tryGetMetaData());

    assert(activeSession_.notNull());
    activeSession_->tryGetFrameData()->dispatcher.removeListener(this);
    activeSession_ = newTraining;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);

}
void DecisionGraphPlugin::onProtocolTrainingEnded(rfcommon::Session* training)
{
    assert(activeSession_.notNull());
    activeSession_->tryGetFrameData()->dispatcher.removeListener(this);
    activeSession_.drop();
}
void DecisionGraphPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    if (state_ != GAME)
        seqSearchModel_->clearAll();
    state_ = GAME;

    if (sessionSettings_->accumulateLiveSessions() == false)
        seqSearchModel_->clearAll();
    seqSearchModel_->startNewSession(game->tryGetMappingInfo(), game->tryGetMetaData());

    assert(activeSession_.isNull());
    activeSession_ = game;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolGameResumed(rfcommon::Session* game) 
{
    if (state_ != GAME)
        seqSearchModel_->clearAll();
    state_ = GAME;

    seqSearchModel_->startNewSession(game->tryGetMappingInfo(), game->tryGetMetaData());
    seqSearchModel_->addAllFrames(game->tryGetFrameData());
    seqSearchModel_->applyAllQueries();

    assert(activeSession_.isNull());
    activeSession_ = game;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolGameEnded(rfcommon::Session* game) 
{
    assert(activeSession_.notNull());
    activeSession_->tryGetFrameData()->dispatcher.removeListener(this);
    activeSession_.drop();
}

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

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onUserMotionLabelsLayerAdded(int layerIdx, const char* name)
{
    seqSearchModel_->applyAllQueries();
}
void DecisionGraphPlugin::onUserMotionLabelsLayerRemoved(int layerIdx, const char* name)
{
    seqSearchModel_->applyAllQueries();
}
void DecisionGraphPlugin::onUserMotionLabelsNewEntry(rfcommon::FighterID fighterID, int entryIdx)
{
    seqSearchModel_->applyAllQueries();
}
void DecisionGraphPlugin::onUserMotionLabelsUserLabelChanged(rfcommon::FighterID fighterID, int entryIdx, const char* oldLabel, const char* newLabel)
{
    seqSearchModel_->applyAllQueries();
}
void DecisionGraphPlugin::onUserMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int entryIdx, rfcommon::UserMotionLabelsCategory oldCategory, rfcommon::UserMotionLabelsCategory newCategory)
{
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    assert(activeSession_.notNull());

    seqSearchModel_->addFrame(frameIdx, activeSession_->tryGetFrameData());

    if (noNotifyFrames_-- <= 0)
    {
        rfcommon::HighresTimer timer;
        timer.start();
            seqSearchModel_->applyAllQueries();
        timer.stop();

        noNotifyFrames_ = timer.timePassedNS() * 60 / 1e9;
        noNotifyFrames_ *= 2;  // Give some leeway
    }
}

void DecisionGraphPlugin::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onSessionSettingsChanged()
{
    if (sessionSettings_->accumulateLiveSessions() == false && seqSearchModel_->sessionCount() > 1)
        DecisionGraphPlugin::onClearPreviousSessions();
}
void DecisionGraphPlugin::onClearPreviousSessions()
{
    if (seqSearchModel_->sessionCount() <= 1)
        return;

    seqSearchModel_->clearAll();
    if (activeSession_)
    {
        auto map = activeSession_->tryGetMappingInfo();
        auto mdata = activeSession_->tryGetMetaData();
        auto fdata = activeSession_->tryGetFrameData();
        if (map && mdata && fdata)
        {
            seqSearchModel_->startNewSession(map, mdata);
            seqSearchModel_->addAllFrames(fdata);
            seqSearchModel_->applyAllQueries();
        }
    }
}
