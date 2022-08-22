#include "decision-graph/views/TimingsView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

// ----------------------------------------------------------------------------
TimingsView::TimingsView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , labels_(labels)
{
    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
TimingsView::~TimingsView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void TimingsView::onCurrentFighterChanged() {}
void TimingsView::onNewSession() {}
void TimingsView::onDataAdded() {}
void TimingsView::onDataCleared() {}
void TimingsView::onQueryApplied() {}
