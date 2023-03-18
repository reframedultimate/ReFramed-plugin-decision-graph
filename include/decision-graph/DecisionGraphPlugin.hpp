#pragma once

#include "decision-graph/listeners/SessionSettingsListener.hpp"
#include "rfcommon/Plugin.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/MotionLabelsListener.hpp"
#include <memory>

class GraphModel;
class LabelMapper;
class SequenceSearchModel;
class SessionSettingsModel;
class VisualizerModel;

namespace rfcommon {
    class MotionLabels;
}

class DecisionGraphPlugin
        : public rfcommon::Plugin
        , public rfcommon::Plugin::UIInterface
        , public rfcommon::Plugin::RealtimeInterface
        , public rfcommon::Plugin::ReplayInterface
        , public rfcommon::MotionLabelsListener
        , public rfcommon::FrameDataListener
        , public SessionSettingsListener
{
public:
    DecisionGraphPlugin(RFPluginFactory* factory, rfcommon::PluginContext* pluginCtx, rfcommon::MotionLabels* labels);
    ~DecisionGraphPlugin();

private:
    /*!
     * This is called by ReFramed to create an instance of your view.
     * It is possible that this gets called more than once, for example if
     * ReFramed wants to add your view to different parts of the program.
     * Your view should be designed in a way such that multiple views can
     * share the same underlying model.
     */
    QWidget* createView() override final;

    /*!
     * The counter-part to createView(). When ReFramed removes your view
     * it will give it back to you to destroy.
     */
    void destroyView(QWidget* view) override final;

private:
    Plugin::UIInterface* uiInterface() override final;
    Plugin::ReplayInterface* replayInterface() override final;
    Plugin::SharedDataInterface* sharedInterface() override final;
    Plugin::RealtimeInterface* realtimeInterface() override final;
    Plugin::VideoPlayerInterface* videoPlayerInterface() override final;

private:
    void onProtocolAttemptConnectToServer(const char* ipAddress, uint16_t port) override final;
    void onProtocolFailedToConnectToServer(const char* errormsg, const char* ipAddress, uint16_t port) override final;
    void onProtocolConnectedToServer(const char* ipAddress, uint16_t port) override final;
    void onProtocolDisconnectedFromServer() override final;

    void onProtocolTrainingStarted(rfcommon::Session* training) override final;
    void onProtocolTrainingResumed(rfcommon::Session* training) override final;
    void onProtocolTrainingReset(rfcommon::Session* oldTraining, rfcommon::Session* newTraining) override final;
    void onProtocolTrainingEnded(rfcommon::Session* training) override final;
    void onProtocolGameStarted(rfcommon::Session* game) override final;
    void onProtocolGameResumed(rfcommon::Session* game) override final;
    void onProtocolGameEnded(rfcommon::Session* game) override final;

private:
    void onGameSessionLoaded(rfcommon::Session* game) override final;
    void onGameSessionUnloaded(rfcommon::Session* game) override final;
    void onTrainingSessionLoaded(rfcommon::Session* training) override final;
    void onTrainingSessionUnloaded(rfcommon::Session* training) override final;

    void onGameSessionSetLoaded(rfcommon::Session** games, int numGames) override final;
    void onGameSessionSetUnloaded(rfcommon::Session** games, int numGames) override final;

private:
    void onMotionLabelsLoaded() override;
    void onMotionLabelsHash40sUpdated() override;

    void onMotionLabelsPreferredLayerChanged(int usage) override;

    void onMotionLabelsLayerInserted(int layerIdx) override;
    void onMotionLabelsLayerRemoved(int layerIdx) override;
    void onMotionLabelsLayerNameChanged(int layerIdx) override;
    void onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) override;
    void onMotionLabelsLayerMoved(int fromIdx, int toIdx) override;
    void onMotionLabelsLayerMerged(int layerIdx) override;

    void onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) override;
    void onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) override;
    void onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) override;

private:
    void onFrameDataNewUniqueFrame(int frameIdx, const rfcommon::Frame<4>& frame) override final;
    void onFrameDataNewFrame(int frameIdx, const rfcommon::Frame<4>& frame) override final;

private:
    void onSessionSettingsChanged() override final;
    void onClearPreviousSessions() override final;

private:
    std::unique_ptr<GraphModel> graphModel_;
    std::unique_ptr<SequenceSearchModel> seqSearchModel_;
    std::unique_ptr<SessionSettingsModel> sessionSettings_;
    std::unique_ptr<VisualizerModel> visualizerModel_;
    rfcommon::Reference<rfcommon::Session> activeSession_;
    rfcommon::MotionLabels* labels_;
    int noNotifyFrames_ = 0;

    enum State
    {
        NONE,
        GAME,
        TRAINING,
        REPLAY
    } state_ = NONE;
};
