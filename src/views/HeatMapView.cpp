#include "decision-graph/views/HeatMapView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

// ----------------------------------------------------------------------------
HeatMapView::HeatMapView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , labels_(labels)
{
    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
HeatMapView::~HeatMapView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void HeatMapView::onCurrentFighterChanged() {}
void HeatMapView::onNewSession() {}
void HeatMapView::onDataAdded() {}
void HeatMapView::onDataCleared() {}
void HeatMapView::onQueryApplied() {}
