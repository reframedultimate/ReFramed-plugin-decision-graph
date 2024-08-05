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

class QPieSeries;
class QBarSeries;
class QBarSet;
class QStackedBarSeries;
class QBarCategoryAxis;
class QValueAxis;

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
    void onNewSessions() override;
    void onClearAll() override;
    void onDataAdded() override;
    void onPOVChanged() override;
    void onQueriesChanged() override;
    void onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) override;
    void onQueriesApplied() override;

private:
    SequenceSearchModel* model_;
    rfcommon::MotionLabels* labels_;
    QStackedWidget* pieStack_;

    QPieSeries* pieIncomingSeries_;
    QPieSeries* pieOutgoingSeries_;

    QPieSeries* pieBreakdownSeries_;

    QwtPlot* barBreakdownPlot_;
    QwtPlotMultiBarChart* barBreakdownData_;
};
