#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "rfcommon/Plugin.hpp"

class SequenceSearchModel;

extern "C" {
    struct RFPluginFactory;
}

namespace rfcommon {
    class VisualizerContext;
}

class VisualizerModel
        : public rfcommon::Plugin::VisualizerInterface
        , public SequenceSearchListener
{
public:
    VisualizerModel(SequenceSearchModel* seqSearchModel, rfcommon::VisualizerContext* visCtx, RFPluginFactory* factory);
    ~VisualizerModel();

private:
    void updateData();

private:
    void onVisualizerDataChanged() override;

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
