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
    void onSharedDataChanged() override;

private:
    void onNewSessions() override;
    void onClearAll() override;
    void onDataAdded() override;
    void onPOVChanged() override;
    void onQueriesChanged() override;
    void onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) override;
    void onQueriesApplied() override;

private:
    SequenceSearchModel* seqSearchModel_;
};
