#pragma once

#include <QWidget>

class SequenceSearchModel;

namespace QtCharts {
    class QChartView;
}

class PieChartView : public QWidget
{
public:
    explicit PieChartView(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PieChartView();

    void updatePieCharts();
    void updateBarChart();
    void updateStackedBarChart();

private:
    SequenceSearchModel* model_;
    QtCharts::QChartView* pieChartView_;
};
