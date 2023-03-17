#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class SequenceSearchModel;

namespace rfcommon {
    class MotionLabels;
}

class HeatMapView
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit HeatMapView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent=nullptr);
    ~HeatMapView();

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryCompiled(int queryIdx) override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    rfcommon::MotionLabels* labels_;
};
