#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class LabelMapper;
class SequenceSearchModel;

class HeatMapView
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit HeatMapView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent=nullptr);
    ~HeatMapView();

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    LabelMapper* labels_;
};
