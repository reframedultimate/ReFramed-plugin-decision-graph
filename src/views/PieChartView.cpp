#include "decision-graph/views/PieChartView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

#include <QVBoxLayout>

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>
#include <QtCharts/QBarSet>
#include <QtCharts/QStackedBarSeries>

// ----------------------------------------------------------------------------
PieChartView::PieChartView(SequenceSearchModel* model, QWidget* parent)
    : model_(model)
{
    QtCharts::QPieSeries* series = new QtCharts::QPieSeries();
    series->append("Jane", 1);
    series->append("Joe", 2);
    series->append("Andy", 3);
    series->append("Barbara", 4);
    series->append("Axel", 5);
    series->setLabelsVisible(true);

    QtCharts::QPieSlice* slice = series->slices().at(1);
    slice->setExploded();
    slice->setLabelVisible();
    slice->setPen(QPen(Qt::darkGreen, 2));
    slice->setBrush(Qt::green);

    QtCharts::QChart* chart = new QtCharts::QChart();
    chart->addSeries(series);
    chart->setTitle("Option Breakdown");
    chart->legend()->hide();

    QtCharts::QChartView* chartView = new QtCharts::QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    /*
    QBarSet* set0 = new QBarSet("Jane");
    QBarSet* set1 = new QBarSet("John");
    QBarSet* set2 = new QBarSet("Axel");
    QBarSet* set3 = new QBarSet("Mary");
    QBarSet* set4 = new QBarSet("Samantha");

    *set0 << 1 << 2 << 3 << 4 << 5 << 6;
    *set1 << 5 << 0 << 0 << 4 << 0 << 7;
    *set2 << 3 << 5 << 8 << 13 << 8 << 5;
    *set3 << 5 << 6 << 7 << 3 << 4 << 5;
    *set4 << 9 << 7 << 5 << 3 << 1 << 2;

    QStackedBarSeries* series = new QStackedBarSeries();
    series->append(set0);
    series->append(set1);
    series->append(set2);
    series->append(set3);
    series->append(set4);

    QChart* chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Simple stackedbarchart example");

    QStringList categories;
    categories << "Jan" << "Feb" << "Mar" << "Apr" << "May" << "Jun";
    QBarCategoryAxis* axisX = new QBarCategoryAxis();
    axisX->append(categories);
    chart->addAxis(axisX, Qt::AlignBottom);
    series->attachAxis(axisX);
    QValueAxis* axisY = new QValueAxis();
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisY);

    chart->legend()->setVisible(true);
    chart->legend()->setAlignment(Qt::AlignBottom);

    QChartView* chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    ui_->tab_stateList->setLayout(new QVBoxLayout);
    ui_->tab_stateList->layout()->addWidget(chartView);*/

    QGridLayout* chartsLayout = new QGridLayout;
    chartsLayout->addWidget(chartView);

    setLayout(chartsLayout);
}

// ----------------------------------------------------------------------------
PieChartView::~PieChartView()
{}

// ----------------------------------------------------------------------------
void PieChartView::updatePieCharts()
{
    // We have two modes of operation. If the user only ran one query, we try
    // to guess all of the different options based on incoming and outgoing
    // connections in the decision graph
    //
    // If the user ran multiple queries, then we simply use the number of
    // matches for each query
}

// ----------------------------------------------------------------------------
void PieChartView::updateBarChart()
{

}

// ----------------------------------------------------------------------------
void PieChartView::updateStackedBarChart()
{

}
