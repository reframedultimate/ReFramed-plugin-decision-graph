#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class LabelMapper;
class SequenceSearchModel;

class QwtPlot;
class QwtPlotHistogram;

class TimingsView
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit TimingsView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent=nullptr);
    ~TimingsView();

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    LabelMapper* labels_;
    QwtPlot* relativePlot_;
    QwtPlotHistogram* relativeData_;
};
