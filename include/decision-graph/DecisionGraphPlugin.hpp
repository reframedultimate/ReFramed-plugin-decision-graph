#pragma once

#include "rfcommon/Plugin.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include <memory>

class GraphModel;
class LabelMapper;
class SequenceSearchModel;

namespace rfcommon {
    class UserMotionLabels;
    class Hash40Strings;
}

class DecisionGraphPlugin
        : public rfcommon::Plugin
        , public rfcommon::Plugin::UIInterface
        , public rfcommon::Plugin::RealtimeInterface
        , public rfcommon::Plugin::ReplayInterface
{
public:
    DecisionGraphPlugin(RFPluginFactory* factory, rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings);
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
    Plugin::VisualizerInterface* visualizerInterface() override final;
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
    std::unique_ptr<LabelMapper> labelMapper_;
    std::unique_ptr<GraphModel> graphModel_;
    std::unique_ptr<SequenceSearchModel> seqSearchModel_;
};
