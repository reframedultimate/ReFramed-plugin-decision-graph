#include "decision-graph/views/DamageView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

// ----------------------------------------------------------------------------
DamageView::DamageView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , labels_(labels)
{
    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
DamageView::~DamageView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void DamageView::onPOVChanged() {}
void DamageView::onNewSession() {}
void DamageView::onDataAdded() {}
void DamageView::onDataCleared() {}
void DamageView::onQueryCompiled(int queryIdx) {}
void DamageView::onQueryApplied() {}
