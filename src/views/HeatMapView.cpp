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
void HeatMapView::onNewSessions() {}
void HeatMapView::onClearAll() {}
void HeatMapView::onDataAdded() {}
void HeatMapView::onPOVChanged() {}
void HeatMapView::onQueriesChanged() {}
void HeatMapView::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void HeatMapView::onQueriesApplied() {}
