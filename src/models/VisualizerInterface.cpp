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
void VisualizerModel::updateData()
{
    rfcommon::PluginSharedData data;
    const auto& states = seqSearchModel_->fighterStates(seqSearchModel_->currentFighter());
    for (int queryIdx = 0; queryIdx != seqSearchModel_->queryCount(); ++queryIdx)
    {
        rfcommon::Vector<rfcommon::PluginSharedData::TimeInterval> timeIntervals;
        for (const auto range : seqSearchModel_->matches(queryIdx))
        {
            assert(range.startIdx != range.endIdx);
            const char* name = seqSearchModel_->queryStr(queryIdx);
            const auto startFrame = states[range.startIdx].sideData.frameIndex;
            const auto endFrame = states[range.endIdx - 1].sideData.frameIndex;
            timeIntervals.emplace(name, startFrame, rfcommon::FrameIndex::fromValue(endFrame.index() + 1));
        }
        data.timeIntervalSets.insertAlways(seqSearchModel_->queryStr(queryIdx), std::move(timeIntervals));
    }
    setSharedData(std::move(data));
}

// ----------------------------------------------------------------------------
void VisualizerModel::onSharedDataChanged() {}

// ----------------------------------------------------------------------------
void VisualizerModel::onPOVChanged() { updateData(); }
void VisualizerModel::onNewSession() { updateData(); }
void VisualizerModel::onDataAdded() { updateData(); }
void VisualizerModel::onDataCleared() { updateData(); }
void VisualizerModel::onQueryCompiled(int queryIdx) {}
void VisualizerModel::onQueryApplied() { updateData(); }
