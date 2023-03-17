#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "rfcommon/Plugin.hpp"

class SequenceSearchModel;

extern "C" {
    struct RFPluginFactory;
}

namespace rfcommon {
    class PluginContext;
}

class VisualizerModel
        : public rfcommon::Plugin::SharedDataInterface
        , public SequenceSearchListener
{
public:
    VisualizerModel(SequenceSearchModel* seqSearchModel, rfcommon::PluginContext* pluginCtx, RFPluginFactory* factory);
    ~VisualizerModel();

private:
    void updateData();

private:
    void onSharedDataChanged() override;

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryCompiled(int queryIdx) override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* seqSearchModel_;
};
