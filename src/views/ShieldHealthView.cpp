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
void ShieldHealthView::onNewSessions() {}
void ShieldHealthView::onClearAll() {}
void ShieldHealthView::onDataAdded() {}
void ShieldHealthView::onPOVChanged() {}
void ShieldHealthView::onQueriesChanged() {}
void ShieldHealthView::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void ShieldHealthView::onQueriesApplied() {}
