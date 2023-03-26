#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class SequenceSearchModel;

class QStackedWidget;
class QwtPlot;
class QwtPlotMultiBarChart;

namespace rfcommon {
    class MotionLabels;
}

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
    explicit PieChartView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent=nullptr);
    ~PieChartView();

private:
    void updateVisible();
    void updateIOCharts();
    void updateBreakdownCharts();

private:
    void onPOVChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryCompiled(int queryIdx) override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    rfcommon::MotionLabels* labels_;
    QStackedWidget* pieStack_;

    QtCharts::QPieSeries* pieIncomingSeries_;
    QtCharts::QPieSeries* pieOutgoingSeries_;

    QtCharts::QPieSeries* pieBreakdownSeries_;

    QwtPlot* barBreakdownPlot_;
    QwtPlotMultiBarChart* barBreakdownData_;
};
