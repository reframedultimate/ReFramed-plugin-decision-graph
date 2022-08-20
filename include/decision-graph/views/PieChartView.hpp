#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class SequenceSearchModel;
class LabelMapper;

class QStackedWidget;

class QwtPlot;
class QwtPlotMultiBarChart;

namespace QtCharts {
    class QPieSeries;
    class QBarSeries;
    class QBarSet;
    class QStackedBarSeries;
    class QBarCategoryAxis;
    class QValueAxis;
}

class PieChartView 
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit PieChartView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent=nullptr);
    ~PieChartView();

private:
    void updateIOCharts();
    void updateBreakdownCharts();

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    LabelMapper* labels_;
    QStackedWidget* pieStack_;
    QStackedWidget* barStack_;

    QtCharts::QPieSeries* pieIncomingSeries_;
    QtCharts::QPieSeries* pieOutgoingSeries_;

    QwtPlot* barIncomingPlot_;
    QwtPlot* barOutgoingPlot_;
    QwtPlotMultiBarChart* barIncomingData_;
    QwtPlotMultiBarChart* barOutgoingData_;

    QtCharts::QPieSeries* pieBreakdownSeries_;

    QwtPlot* barBreakdownPlot_;
    QwtPlotMultiBarChart* barBreakdownData_;
};
