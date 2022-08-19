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
    , pieBreakdownSeries_(new QtCharts::QPieSeries)
    , pieIncomingSeries_(new QtCharts::QPieSeries)
    , pieOutgoingSeries_(new QtCharts::QPieSeries)
{
    QtCharts::QChart* breakdownPieChart = new QtCharts::QChart();
    breakdownPieChart->addSeries(pieBreakdownSeries_);
    breakdownPieChart->setTitle("Option Breakdown");
    breakdownPieChart->legend()->hide();

    pieBreakdownView_ = new QtCharts::QChartView(breakdownPieChart);
    pieBreakdownView_->setRenderHint(QPainter::Antialiasing);

    QtCharts::QChart* incomingPieChart = new QtCharts::QChart();
    incomingPieChart->addSeries(pieIncomingSeries_);
    incomingPieChart->setTitle("Incoming Options");
    incomingPieChart->legend()->hide();

    pieIncomingView_ = new QtCharts::QChartView(incomingPieChart);
    pieIncomingView_->setRenderHint(QPainter::Antialiasing);

    QtCharts::QChart* outgoingPieChart = new QtCharts::QChart();
    outgoingPieChart->addSeries(pieOutgoingSeries_);
    outgoingPieChart->setTitle("Outgoing Options");
    outgoingPieChart->legend()->hide();

    pieOutgoingView_ = new QtCharts::QChartView(outgoingPieChart);
    pieOutgoingView_->setRenderHint(QPainter::Antialiasing);

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
    chartsLayout->addWidget(pieBreakdownView_, 0, 0, 2, 1);
    chartsLayout->addWidget(pieIncomingView_, 1, 0, 1, 1);
    chartsLayout->addWidget(pieOutgoingView_, 1, 1, 1, 1);

    setLayout(chartsLayout);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
PieChartView::~PieChartView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void PieChartView::updateBreakdownPieChart()
{
    // We have two modes of operation. If the user only ran one query, we try
    // to guess all of the different options based on incoming and outgoing
    // connections in the decision graph
    //
    // If the user ran multiple queries, then we simply use the number of
    // matches for each query

    pieBreakdownSeries_->clear();

    if (model_->queryCount() > 1)
    {
        for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
            pieBreakdownSeries_->append(model_->queryStr(queryIdx), model_->matches(queryIdx).count());

        pieBreakdownSeries_->setLabelsVisible(true);
        pieBreakdownView_->setVisible(true);
    }

    if (model_->queryCount() == 1)
    {
        pieBreakdownView_->setVisible(false);
    }
}

// ----------------------------------------------------------------------------
void PieChartView::updateIOPieCharts()
{
    // We have two modes of operation. If the user only ran one query, we try
    // to guess all of the different options based on incoming and outgoing
    // connections in the decision graph
    //
    // If the user ran multiple queries, then we simply use the number of
    // matches for each query

    pieOutgoingSeries_->clear();
    pieIncomingSeries_->clear();

    if (model_->queryCount() == 1)
    {
        // Find largest island
        const auto islands = model_->graph(0).islands();
        int largest = 0;
        int nodes = 0;
        for (int i = 0; i != islands.count(); ++i)
            if (nodes < islands[i].nodes.count())
            {
                nodes = islands[i].nodes.count();
                largest = i;
            }

        const Graph& graph = islands[largest];

        for (const auto& unique : graph.cutLoopsIncoming().uniqueSinks())
            pieOutgoingSeries_->append(unique.sequence.toString().cStr(), unique.weight);
        pieOutgoingSeries_->setLabelsVisible(true);

        for (const auto& unique : graph.cutLoopsOutgoing().uniqueSources())
            pieIncomingSeries_->append(unique.sequence.toString().cStr(), unique.weight);
        pieIncomingSeries_->setLabelsVisible(true);

        pieIncomingView_->setVisible(true);
        pieOutgoingView_->setVisible(true);
    }

    if (model_->queryCount() > 1)
    {
        pieIncomingView_->setVisible(false);
        pieOutgoingView_->setVisible(false);
    }
}

// ----------------------------------------------------------------------------
void PieChartView::updateBarChart()
{

}

// ----------------------------------------------------------------------------
void PieChartView::updateStackedBarChart()
{

}

// ----------------------------------------------------------------------------
void PieChartView::onCurrentFighterChanged()
{
}

// ----------------------------------------------------------------------------
void PieChartView::onNewSession()
{
}

// ----------------------------------------------------------------------------
void PieChartView::onDataAdded()
{
}

// ----------------------------------------------------------------------------
void PieChartView::onDataCleared()
{
}

// ----------------------------------------------------------------------------
void PieChartView::onQueryApplied()
{
    updateBreakdownPieChart();
    updateIOPieCharts();
}

