#include "decision-graph/views/ShieldHealthView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

// ----------------------------------------------------------------------------
ShieldHealthView::ShieldHealthView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent)
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
void ShieldHealthView::onCurrentFighterChanged() {}
void ShieldHealthView::onNewSession() {}
void ShieldHealthView::onDataAdded() {}
void ShieldHealthView::onDataCleared() {}
void ShieldHealthView::onQueryApplied() {}
