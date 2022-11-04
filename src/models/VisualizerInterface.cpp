#include "decision-graph/models/VisualizerInterface.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "rfcommon/VisualizerData.hpp"

// ----------------------------------------------------------------------------
VisualizerModel::VisualizerModel(SequenceSearchModel* seqSearchModel, rfcommon::VisualizerContext* visCtx, RFPluginFactory* factory)
    : VisualizerInterface(visCtx, factory)
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
    rfcommon::VisualizerData data;
    const auto& states = seqSearchModel_->fighterStates(seqSearchModel_->currentFighter());
    for (int queryIdx = 0; queryIdx != seqSearchModel_->queryCount(); ++queryIdx)
        for (const auto range : seqSearchModel_->matches(queryIdx))
        {
            assert(range.startIdx != range.endIdx);
            const char* name = seqSearchModel_->queryStr(queryIdx);
            const auto startFrame = states[range.startIdx].sideData.frameIndex;
            const auto endFrame = states[range.endIdx - 1].sideData.frameIndex;
            data.timeIntervals.emplace(name, startFrame, rfcommon::FrameIndex::fromValue(endFrame.index() + 1));
        }
    setVisualizerData(std::move(data));
}

// ----------------------------------------------------------------------------
void VisualizerModel::onVisualizerDataChanged()
{}

// ----------------------------------------------------------------------------
void VisualizerModel::onCurrentFighterChanged() { updateData(); }
void VisualizerModel::onNewSession() { updateData(); }
void VisualizerModel::onDataAdded() { updateData(); }
void VisualizerModel::onDataCleared() { updateData(); }
void VisualizerModel::onQueryCompiled(int queryIdx) {}
void VisualizerModel::onQueryApplied() { updateData(); }
