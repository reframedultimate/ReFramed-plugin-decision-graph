#include "decision-graph/views/ShieldHealthView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

// ----------------------------------------------------------------------------
ShieldHealthView::ShieldHealthView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , labels_(labels)
{
    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
ShieldHealthView::~ShieldHealthView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void ShieldHealthView::onPOVChanged() {}
void ShieldHealthView::onNewSession() {}
void ShieldHealthView::onDataAdded() {}
void ShieldHealthView::onDataCleared() {}
void ShieldHealthView::onQueryCompiled(int queryIdx) {}
void ShieldHealthView::onQueryApplied() {}
