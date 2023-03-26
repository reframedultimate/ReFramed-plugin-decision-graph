#include "decision-graph/views/HeatMapView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

// ----------------------------------------------------------------------------
HeatMapView::HeatMapView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent)
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
void HeatMapView::onPOVChanged() {}
void HeatMapView::onNewSession() {}
void HeatMapView::onDataAdded() {}
void HeatMapView::onDataCleared() {}
void HeatMapView::onQueryCompiled(int queryIdx) {}
void HeatMapView::onQueryApplied() {}
