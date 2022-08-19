#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class SequenceSearchModel;

namespace QtCharts {
    class QChartView;
    class QPieSeries;
}

class PieChartView 
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit PieChartView(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PieChartView();

private:
    void updateBreakdownPieChart();
    void updateIOPieCharts();
    void updateBarChart();
    void updateStackedBarChart();

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    QtCharts::QChartView* pieBreakdownView_;
    QtCharts::QPieSeries* pieBreakdownSeries_;
    QtCharts::QChartView* pieIncomingView_;
    QtCharts::QPieSeries* pieIncomingSeries_;
    QtCharts::QChartView* pieOutgoingView_;
    QtCharts::QPieSeries* pieOutgoingSeries_;
};
