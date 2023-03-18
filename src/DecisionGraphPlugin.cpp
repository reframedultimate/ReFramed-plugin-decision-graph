#include "decision-graph/DecisionGraphPlugin.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/Query.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/VisualizerInterface.hpp"

#include "rfcommon/FrameData.hpp"
#include "rfcommon/HighresTimer.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/ReplayFilename.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
DecisionGraphPlugin::DecisionGraphPlugin(RFPluginFactory* factory, rfcommon::PluginContext* pluginCtx, rfcommon::MotionLabels* labels)
    : Plugin(factory)
    , graphModel_(new GraphModel)
    , seqSearchModel_(new SequenceSearchModel(labels))
    , visualizerModel_(new VisualizerModel(seqSearchModel_.get(), pluginCtx, factory))
    , labels_(labels)
{
    labels_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
DecisionGraphPlugin::~DecisionGraphPlugin()
{
    labels_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
rfcommon::Plugin::UIInterface* DecisionGraphPlugin::uiInterface() { return this; }
rfcommon::Plugin::RealtimeInterface* DecisionGraphPlugin::realtimeInterface() { return this; }
rfcommon::Plugin::ReplayInterface* DecisionGraphPlugin::replayInterface() { return this; }
rfcommon::Plugin::SharedDataInterface* DecisionGraphPlugin::sharedInterface() { return visualizerModel_.get(); }
rfcommon::Plugin::VideoPlayerInterface* DecisionGraphPlugin::videoPlayerInterface() { return nullptr; }

// ----------------------------------------------------------------------------
QWidget* DecisionGraphPlugin::createView()
{
    // Create new instance of view. The view registers as a listener to this model
    return new SequenceSearchView(seqSearchModel_.get(), graphModel_.get(), labels_);
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
    seqSearchModel_->clearAll();
    seqSearchModel_->startNewSession(training->tryGetMappingInfo(), training->tryGetMetadata());

    assert(activeSession_.isNull());
    activeSession_ = training;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    seqSearchModel_->clearAll();
    seqSearchModel_->startNewSession(training->tryGetMappingInfo(), training->tryGetMetadata());
    seqSearchModel_->addAllFrames(training->tryGetFrameData());
    seqSearchModel_->applyAllQueries();

    assert(activeSession_.isNull());
    activeSession_ = training;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    seqSearchModel_->clearAll();
    seqSearchModel_->startNewSession(newTraining->tryGetMappingInfo(), newTraining->tryGetMetadata());

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
    seqSearchModel_->clearAll();
    seqSearchModel_->startNewSession(game->tryGetMappingInfo(), game->tryGetMetadata());

    assert(activeSession_.isNull());
    activeSession_ = game;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    seqSearchModel_->clearAll();
    seqSearchModel_->startNewSession(game->tryGetMappingInfo(), game->tryGetMetadata());
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
        if (auto mdata = game->tryGetMetadata())
            if (auto fdata = game->tryGetFrameData())
            {
                seqSearchModel_->startNewSession(map, mdata);
                seqSearchModel_->addAllFrames(fdata);
                seqSearchModel_->applyAllQueries();
            }
}
void DecisionGraphPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    seqSearchModel_->clearAll();
}
void DecisionGraphPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    seqSearchModel_->clearAll();
    if (auto map = training->tryGetMappingInfo())
        if (auto mdata = training->tryGetMetadata())
            if (auto fdata = training->tryGetFrameData())
            {
                seqSearchModel_->startNewSession(map, mdata);
                seqSearchModel_->applyAllQueries();
                seqSearchModel_->addAllFrames(fdata);
            }
}
void DecisionGraphPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    seqSearchModel_->clearAll();
}
void DecisionGraphPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    seqSearchModel_->clearAll();
    for (int i = 0; i != numGames; ++i)
        if (auto map = games[i]->tryGetMappingInfo())
            if (auto mdata = games[i]->tryGetMetadata())
                if (auto fdata = games[i]->tryGetFrameData())
                {
                    seqSearchModel_->startNewSession(map, mdata);
                    seqSearchModel_->addAllFrames(fdata);
                }
    seqSearchModel_->applyAllQueries();
}
void DecisionGraphPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    seqSearchModel_->clearAll();
}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onMotionLabelsLoaded() { seqSearchModel_->applyAllQueries(); }
void DecisionGraphPlugin::onMotionLabelsHash40sUpdated() { seqSearchModel_->applyAllQueries(); }

void DecisionGraphPlugin::onMotionLabelsPreferredLayerChanged(int usage) { seqSearchModel_->applyAllQueries(); }

void DecisionGraphPlugin::onMotionLabelsLayerInserted(int layerIdx) { seqSearchModel_->applyAllQueries(); }
void DecisionGraphPlugin::onMotionLabelsLayerRemoved(int layerIdx) { seqSearchModel_->applyAllQueries(); }
void DecisionGraphPlugin::onMotionLabelsLayerNameChanged(int layerIdx) {}
void DecisionGraphPlugin::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) {}
void DecisionGraphPlugin::onMotionLabelsLayerMoved(int fromIdx, int toIdx) { seqSearchModel_->applyAllQueries(); }
void DecisionGraphPlugin::onMotionLabelsLayerMerged(int layerIdx) { seqSearchModel_->applyAllQueries(); }

void DecisionGraphPlugin::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) { seqSearchModel_->applyAllQueries(); }
void DecisionGraphPlugin::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) { seqSearchModel_->applyAllQueries(); }
void DecisionGraphPlugin::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) {}

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
void DecisionGraphPlugin::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}

