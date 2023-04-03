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
    , seqSearchModel_(new SequenceSearchModel(labels))
    , graphModel_(new GraphModel(seqSearchModel_.get(), labels))
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
    seqSearchModel_->clearAllAndNotify();

    seqSearchModel_->startNewSession(training->tryGetMappingInfo(), training->tryGetMetadata());
    seqSearchModel_->notifyNewSessions();

    assert(activeSession_.isNull());
    activeSession_ = training;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolTrainingResumed(rfcommon::Session* training)
{
    seqSearchModel_->clearAllAndNotify();

    seqSearchModel_->startNewSession(training->tryGetMappingInfo(), training->tryGetMetadata());
    seqSearchModel_->notifyNewSessions();

    seqSearchModel_->addAllFrames(training->tryGetFrameData());
    seqSearchModel_->notifyFramesAdded();

    if (seqSearchModel_->applyAllQueries())
        seqSearchModel_->notifyQueriesApplied();

    assert(activeSession_.isNull());
    activeSession_ = training;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining)
{
    seqSearchModel_->clearAllAndNotify();

    seqSearchModel_->startNewSession(newTraining->tryGetMappingInfo(), newTraining->tryGetMetadata());
    seqSearchModel_->notifyNewSessions();

    assert(activeSession_.notNull());
    activeSession_->tryGetFrameData()->dispatcher.removeListener(this);
    activeSession_ = newTraining;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);

}
void DecisionGraphPlugin::onProtocolTrainingEnded(rfcommon::Session* training)
{
    activeSession_->tryGetFrameData()->dispatcher.removeListener(this);
    activeSession_.drop();
}
void DecisionGraphPlugin::onProtocolGameStarted(rfcommon::Session* game)
{
    seqSearchModel_->clearAllAndNotify();

    seqSearchModel_->startNewSession(game->tryGetMappingInfo(), game->tryGetMetadata());
    seqSearchModel_->notifyNewSessions();

    assert(activeSession_.isNull());
    activeSession_ = game;
    activeSession_->tryGetFrameData()->dispatcher.addListener(this);
}
void DecisionGraphPlugin::onProtocolGameResumed(rfcommon::Session* game)
{
    seqSearchModel_->clearAllAndNotify();

    seqSearchModel_->startNewSession(game->tryGetMappingInfo(), game->tryGetMetadata());
    seqSearchModel_->notifyNewSessions();

    seqSearchModel_->addAllFrames(game->tryGetFrameData());
    seqSearchModel_->notifyFramesAdded();

    if (seqSearchModel_->applyAllQueries())
        seqSearchModel_->notifyQueriesApplied();

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
    seqSearchModel_->clearAllAndNotify();

    if (auto map = game->tryGetMappingInfo())
        if (auto mdata = game->tryGetMetadata())
            if (auto fdata = game->tryGetFrameData())
            {
                seqSearchModel_->startNewSession(map, mdata);
                seqSearchModel_->notifyNewSessions();

                seqSearchModel_->addAllFrames(fdata);
                seqSearchModel_->notifyFramesAdded();

                seqSearchModel_->compileAllQueries();

                if (seqSearchModel_->applyAllQueries())
                    seqSearchModel_->notifyQueriesApplied();
            }
}
void DecisionGraphPlugin::onGameSessionUnloaded(rfcommon::Session* game)
{
    seqSearchModel_->clearAllAndNotify();
}
void DecisionGraphPlugin::onTrainingSessionLoaded(rfcommon::Session* training)
{
    seqSearchModel_->clearAllAndNotify();

    if (auto map = training->tryGetMappingInfo())
        if (auto mdata = training->tryGetMetadata())
            if (auto fdata = training->tryGetFrameData())
            {
                seqSearchModel_->startNewSession(map, mdata);
                seqSearchModel_->notifyNewSessions();

                seqSearchModel_->addAllFrames(fdata);
                seqSearchModel_->notifyFramesAdded();

                if (seqSearchModel_->applyAllQueries())
                    seqSearchModel_->notifyQueriesApplied();
            }
}
void DecisionGraphPlugin::onTrainingSessionUnloaded(rfcommon::Session* training)
{
    seqSearchModel_->clearAllAndNotify();
}
void DecisionGraphPlugin::onGameSessionSetLoaded(rfcommon::Session** games, int numGames)
{
    seqSearchModel_->clearAllAndNotify();
    for (int i = 0; i != numGames; ++i)
        if (auto map = games[i]->tryGetMappingInfo())
            if (auto mdata = games[i]->tryGetMetadata())
                if (auto fdata = games[i]->tryGetFrameData())
                {
                    seqSearchModel_->startNewSession(map, mdata);
                    seqSearchModel_->addAllFrames(fdata);
                }

    seqSearchModel_->notifyNewSessions();
    seqSearchModel_->notifyFramesAdded();

    if (seqSearchModel_->applyAllQueries())
        seqSearchModel_->notifyQueriesApplied();
}
void DecisionGraphPlugin::onGameSessionSetUnloaded(rfcommon::Session** games, int numGames)
{
    seqSearchModel_->clearAllAndNotify();
}

// ----------------------------------------------------------------------------
static void recompileAndApplyQueries(SequenceSearchModel* m)
{
    if (m->compileAllQueries())
        if (m->applyAllQueries())
            m->notifyQueriesApplied();
}
void DecisionGraphPlugin::onMotionLabelsLoaded() { recompileAndApplyQueries(seqSearchModel_.get()); }
void DecisionGraphPlugin::onMotionLabelsHash40sUpdated() { recompileAndApplyQueries(seqSearchModel_.get()); }

void DecisionGraphPlugin::onMotionLabelsPreferredLayerChanged(int usage) { recompileAndApplyQueries(seqSearchModel_.get()); }

void DecisionGraphPlugin::onMotionLabelsLayerInserted(int layerIdx) { recompileAndApplyQueries(seqSearchModel_.get()); }
void DecisionGraphPlugin::onMotionLabelsLayerRemoved(int layerIdx) { recompileAndApplyQueries(seqSearchModel_.get()); }
void DecisionGraphPlugin::onMotionLabelsLayerNameChanged(int layerIdx) {}
void DecisionGraphPlugin::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) {}
void DecisionGraphPlugin::onMotionLabelsLayerMoved(int fromIdx, int toIdx) { recompileAndApplyQueries(seqSearchModel_.get()); }
void DecisionGraphPlugin::onMotionLabelsLayerMerged(int layerIdx) { recompileAndApplyQueries(seqSearchModel_.get()); }

void DecisionGraphPlugin::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) { recompileAndApplyQueries(seqSearchModel_.get()); }
void DecisionGraphPlugin::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) { recompileAndApplyQueries(seqSearchModel_.get()); }
void DecisionGraphPlugin::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) {}

// ----------------------------------------------------------------------------
void DecisionGraphPlugin::onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame)
{
    assert(activeSession_.notNull());

    seqSearchModel_->addFrame(frameIdx, activeSession_->tryGetFrameData());
    seqSearchModel_->notifyFramesAdded();

    if (noNotifyFrames_-- <= 0)
    {
        rfcommon::HighresTimer timer;
        timer.start();
            if (seqSearchModel_->applyAllQueries())
                seqSearchModel_->notifyQueriesApplied();
        timer.stop();

        noNotifyFrames_ = timer.timePassedNS() * 60 / 1e9;
        noNotifyFrames_ *= 2;  // Give some leeway
    }
}
void DecisionGraphPlugin::onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) {}

