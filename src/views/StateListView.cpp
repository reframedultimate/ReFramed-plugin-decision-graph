#include "decision-graph/views/StateListView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

// ----------------------------------------------------------------------------
StateListView::StateListView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , labels_(labels)
{
    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
StateListView::~StateListView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void StateListView::onCurrentFighterChanged() {}
void StateListView::onNewSession() {}
void StateListView::onDataAdded() {}
void StateListView::onDataCleared() {}
void StateListView::onQueryCompiled(int queryIdx) {}
void StateListView::onQueryApplied() {}
