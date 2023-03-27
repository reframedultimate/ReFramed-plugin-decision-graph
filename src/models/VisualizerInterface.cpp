#include "decision-graph/models/VisualizerInterface.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

#include "rfcommon/PluginSharedData.hpp"

// ----------------------------------------------------------------------------
VisualizerModel::VisualizerModel(SequenceSearchModel* seqSearchModel, rfcommon::PluginContext* pluginCtx, RFPluginFactory* factory)
    : SharedDataInterface(pluginCtx, factory)
    , seqSearchModel_(seqSearchModel)
{
    seqSearchModel_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
VisualizerModel::~VisualizerModel()
{
    seqSearchModel_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void VisualizerModel::onSharedDataChanged() {}

// ----------------------------------------------------------------------------
void VisualizerModel::onNewSessions() {}
void VisualizerModel::onClearAll()
{
    setSharedData({});
}
void VisualizerModel::onDataAdded() {}
void VisualizerModel::onPOVChanged() {}
void VisualizerModel::onQueriesChanged() {}
void VisualizerModel::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void VisualizerModel::onQueriesApplied()
{
    rfcommon::PluginSharedData data;
    const auto& states = seqSearchModel_->fighterStates(seqSearchModel_->playerPOV());
    for (int queryIdx = 0; queryIdx != seqSearchModel_->queryCount(); ++queryIdx)
    {
        rfcommon::Vector<rfcommon::PluginSharedData::TimeInterval> timeIntervals;
        for (const auto& range : seqSearchModel_->matches(queryIdx))
        {
            assert(range.startIdx != range.endIdx);
            const rfcommon::String& name = seqSearchModel_->playerQuery(queryIdx);
            const auto startFrame = states[range.startIdx].sideData.frameIndex;
            const auto endFrame = states[range.endIdx - 1].sideData.frameIndex;
            timeIntervals.emplace(name, startFrame, rfcommon::FrameIndex::fromValue(endFrame.index() + 1));
        }
        data.timeIntervalSets.insertAlways(seqSearchModel_->playerQuery(queryIdx), std::move(timeIntervals));
    }
    setSharedData(std::move(data));
}
