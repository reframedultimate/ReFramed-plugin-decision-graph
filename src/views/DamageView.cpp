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
void DamageView::onNewSessions() {}
void DamageView::onClearAll() {}
void DamageView::onDataAdded() {}
void DamageView::onPOVChanged() {}
void DamageView::onQueriesChanged() {}
void DamageView::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void DamageView::onQueriesApplied() {}
