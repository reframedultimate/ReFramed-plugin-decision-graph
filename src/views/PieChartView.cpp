#include "decision-graph/views/PieChartView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/LabelMapper.hpp"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QStackedWidget>

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

#include "qwt_plot.h"
#include "qwt_plot_multi_barchart.h"
#include "qwt_text.h"
#include "qwt_column_symbol.h"

#include "qwt_math.h"

// ----------------------------------------------------------------------------
PieChartView::PieChartView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent)
    : model_(model)
    , labels_(labels)
    , pieStack_(new QStackedWidget)
    , barStack_(new QStackedWidget)
    , pieIncomingSeries_(new QtCharts::QPieSeries)
    , pieOutgoingSeries_(new QtCharts::QPieSeries)
    , barIncomingPlot_(new QwtPlot)
    , barOutgoingPlot_(new QwtPlot)
    , barIncomingData_(new QwtPlotMultiBarChart)
    , barOutgoingData_(new QwtPlotMultiBarChart)
    , pieBreakdownSeries_(new QtCharts::QPieSeries)
    , barBreakdownPlot_(new QwtPlot)
    , barBreakdownData_(new QwtPlotMultiBarChart)
{
    {
        QWidget* ioPieCharts = new QWidget;
        ioPieCharts->setLayout(new QHBoxLayout);
        {
            QtCharts::QChart* chart = new QtCharts::QChart();
            chart->addSeries(pieIncomingSeries_);
            chart->setTitle("Incoming Options");
            chart->legend()->hide();

            QtCharts::QChartView* view = new QtCharts::QChartView(chart);
            view->setRenderHint(QPainter::Antialiasing);

            ioPieCharts->layout()->addWidget(view);
        }
        {
            QtCharts::QChart* chart = new QtCharts::QChart();
            chart->addSeries(pieOutgoingSeries_);
            chart->setTitle("Outgoing Options");
            chart->legend()->hide();

            QtCharts::QChartView* view = new QtCharts::QChartView(chart);
            view->setRenderHint(QPainter::Antialiasing);

            ioPieCharts->layout()->addWidget(view);
        }

        pieStack_->addWidget(ioPieCharts);
    }

    {
        QWidget* ioBarCharts = new QWidget;
        ioBarCharts->setLayout(new QHBoxLayout);
        {
            barIncomingPlot_->setPalette(Qt::white);
            barIncomingPlot_->setTitle("Incoming Options");
            barIncomingPlot_->setAxisVisible(QwtPlot::xBottom, false);

            barIncomingData_->setLayoutPolicy(QwtPlotMultiBarChart::AutoAdjustSamples);
            barIncomingData_->setSpacing(3);
            barIncomingData_->setMargin(3);
            barIncomingData_->attach(barIncomingPlot_);
            ioBarCharts->layout()->addWidget(barIncomingPlot_);
        }
        {
            barOutgoingPlot_->setPalette(Qt::white);
            barOutgoingPlot_->setTitle("Outgoing Options");
            barOutgoingPlot_->setAxisVisible(QwtPlot::xBottom, false);

            barOutgoingData_->setLayoutPolicy(QwtPlotMultiBarChart::AutoAdjustSamples);
            barOutgoingData_->setSpacing(3);
            barOutgoingData_->setMargin(3);
            barOutgoingData_->attach(barOutgoingPlot_);
            ioBarCharts->layout()->addWidget(barOutgoingPlot_);
        }

        barStack_->addWidget(ioBarCharts);
    }

    {
        QtCharts::QChart* chart = new QtCharts::QChart();
        chart->addSeries(pieBreakdownSeries_);
        chart->setTitle("Option Breakdown");
        chart->legend()->hide();

        QtCharts::QChartView* view = new QtCharts::QChartView(chart);
        view->setRenderHint(QPainter::Antialiasing);

        pieStack_->addWidget(view);
    }

    {
        barBreakdownPlot_->setPalette(Qt::white);
        barBreakdownPlot_->setTitle("Option Breakdown");
        barBreakdownPlot_->setAxisVisible(QwtPlot::xBottom, false);

        barBreakdownData_->setLayoutPolicy(QwtPlotMultiBarChart::AutoAdjustSamples);
        barBreakdownData_->setSpacing(3);
        barBreakdownData_->setMargin(3);
        barBreakdownData_->attach(barBreakdownPlot_);

        barStack_->addWidget(barBreakdownPlot_);
    }

    setLayout(new QVBoxLayout);
    layout()->addWidget(pieStack_);
    layout()->addWidget(barStack_);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
PieChartView::~PieChartView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void PieChartView::updateIOCharts()
{
    // We have two modes of operation. If the user only ran one query, we try
    // to guess all of the different options based on incoming and outgoing
    // connections in the decision graph
    //
    // If the user ran multiple queries, then we simply use the number of
    // matches for each query

    pieOutgoingSeries_->clear();
    pieIncomingSeries_->clear();
    barOutgoingData_->setSamples({});
    barIncomingData_->setSamples({});

    if (model_->queryCount() == 1)
    {
        // Find largest island
        const auto islands = model_->graph(0).islands();
        if (islands.count() == 0)
            return;

        int largest = 0;
        int nodes = 0;
        for (int i = 0; i != islands.count(); ++i)
            if (nodes < islands[i].nodes.count())
            {
                nodes = islands[i].nodes.count();
                largest = i;
            }

        const Graph& graph = islands[largest];
        const Graph& outgoingTree = graph.outgoingTree();
        const Graph& incomingTree = graph.incomingTree();
        const auto& incomingSequences = incomingTree.treeToUniqueIncomingSequences();
        const auto& outgoingSequences = outgoingTree.treeToUniuqeOutgoingSequences();

        const auto fighterID = model_->fighterID(model_->currentFighter());

        graph.exportDOT("largest_island.dot", model_->fighterID(model_->currentFighter()), labels_);
        outgoingTree.exportDOT("outgoing_tree.dot", fighterID, labels_);
        incomingTree.exportDOT("incoming_tree.dot", fighterID, labels_);

        {
            int highestWeight = 0;
            QVector<QVector<double>> barValues(1);
            QList<QwtText> barTitles;
            for (int i = 0; i != outgoingSequences.count(); ++i)
            {
                const auto& unique = outgoingSequences[i];
                const auto label = unique.sequence.toString(fighterID, labels_);
                pieOutgoingSeries_->append(label.cStr(), unique.weight);

                barValues[0] += (double)unique.weight;
                barTitles += QwtText(label.cStr());

                QColor color = outgoingSequences.count() == 1 ?
                    QColor::fromHsv(200, 219, 64) : i * 2 < outgoingSequences.count() ?
                    QColor::fromHsv(200, i * (219 - 30) * 2 / outgoingSequences.count() + 30, 255) :
                    QColor::fromHsv(200, 219, (outgoingSequences.count() - i - 1) * 2 * (255 - 64) / outgoingSequences.count() + 64);

                auto symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
                symbol->setLineWidth(2);
                symbol->setFrameStyle(QwtColumnSymbol::Raised);
                symbol->setPalette(color);
                barOutgoingData_->setSymbol(i, symbol);

                if (highestWeight < unique.weight)
                    highestWeight = unique.weight;
            }
            pieOutgoingSeries_->setLabelsVisible(true);

            barOutgoingData_->setStyle(QwtPlotMultiBarChart::Grouped);
            barOutgoingData_->setBarTitles(barTitles);
            barOutgoingData_->setSamples(barValues);
            barOutgoingData_->setLegendIconSize(QSize(10, 14));

            barOutgoingPlot_->setAxisScale(QwtPlot::yLeft, 0, highestWeight * 1.05);
            barOutgoingPlot_->setAxisScale(QwtPlot::xBottom, -0.55, 0.55);
        }

        {
            int highestWeight = 0;
            QVector<QVector<double>> barValues(1);
            QList<QwtText> barTitles;
            for (int i = 0; i != incomingSequences.count(); ++i)
            {
                const auto& unique = incomingSequences[i];
                const auto label = unique.sequence.toString(fighterID, labels_);
                pieIncomingSeries_->append(label.cStr(), unique.weight);

                barValues[0] += (double)unique.weight;
                barTitles += QwtText(label.cStr());

                QColor color = incomingSequences.count() == 1 ?
                    QColor::fromHsv(200, 219, 64) : i * 2 < incomingSequences.count() ?
                    QColor::fromHsv(200, i * (219 - 30) * 2 / incomingSequences.count() + 30, 255) :
                    QColor::fromHsv(200, 219, (incomingSequences.count() - i - 1) * 2 * (255 - 64) / incomingSequences.count() + 64);

                auto symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
                symbol->setLineWidth(2);
                symbol->setFrameStyle(QwtColumnSymbol::Raised);
                symbol->setPalette(color);
                barIncomingData_->setSymbol(i, symbol);

                if (highestWeight < unique.weight)
                    highestWeight = unique.weight;
            }
            pieIncomingSeries_->setLabelsVisible(true);

            barIncomingData_->setStyle(QwtPlotMultiBarChart::Grouped);
            barIncomingData_->setBarTitles(barTitles);
            barIncomingData_->setSamples(barValues);
            barIncomingData_->setLegendIconSize(QSize(10, 14));

            barIncomingPlot_->setAxisScale(QwtPlot::yLeft, 0, highestWeight * 1.05);
            barIncomingPlot_->setAxisScale(QwtPlot::xBottom, -0.55, 0.55);
        }
    }

    barOutgoingPlot_->replot();
    barIncomingPlot_->replot();
}

// ----------------------------------------------------------------------------
void PieChartView::updateBreakdownCharts()
{
    // We have two modes of operation. If the user only ran one query, we try
    // to guess all of the different options based on incoming and outgoing
    // connections in the decision graph
    //
    // If the user ran multiple queries, then we simply use the number of
    // matches for each query

    pieBreakdownSeries_->clear();
    barBreakdownData_->setSamples({});

    if (model_->queryCount() > 1)
    {
        for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
        {
            const auto label = model_->queryStr(queryIdx);
            const int value = model_->matches(queryIdx).count();

            pieBreakdownSeries_->append(label, value);
        }
        pieBreakdownSeries_->setLabelsVisible(true);

        int highestValue = 0;
        QVector<QVector<double>> barValues;
        for (int sessionIdx = 0; sessionIdx != model_->sessionCount(); ++sessionIdx)
        {
            QVector<double> values;
            int valueStacked = 0;
            for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
            {
                const int value = model_->sessionMatches(queryIdx, sessionIdx).count();
                values += value;
                valueStacked += value;
            }
            barValues += values;

            if (highestValue < valueStacked)
                highestValue = valueStacked;
        }

        QList<QwtText> barTitles;
        for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
        {
            QColor color = model_->queryCount() == 1 ?
                QColor::fromHsv(200, 219, 64) : queryIdx * 2 < model_->queryCount() ?
                QColor::fromHsv(200, queryIdx * (219 - 30) * 2 / model_->queryCount() + 30, 255) :
                QColor::fromHsv(200, 219, (model_->queryCount() - queryIdx - 1) * 2 * (255 - 64) / model_->queryCount() + 64);

            auto symbol = new QwtColumnSymbol(QwtColumnSymbol::Box);
            symbol->setLineWidth(2);
            symbol->setFrameStyle(QwtColumnSymbol::Raised);
            symbol->setPalette(color);
            barBreakdownData_->setSymbol(queryIdx, symbol);

            barTitles += QwtText(model_->queryStr(queryIdx));
        }

        if (model_->sessionCount() == 1)
        {
            barBreakdownData_->setStyle(QwtPlotMultiBarChart::Grouped);
            barBreakdownPlot_->setAxisScale(QwtPlot::xBottom, -0.55, 0.55);
        }
        else
        {
            barBreakdownData_->setStyle(QwtPlotMultiBarChart::Stacked);
            barBreakdownPlot_->setAxisAutoScale(QwtPlot::xBottom);
        }

        barBreakdownData_->setBarTitles(barTitles);
        barBreakdownData_->setSamples(barValues);
        barBreakdownData_->setLegendIconSize(QSize(10, 14));

        barBreakdownPlot_->setAxisScale(QwtPlot::yLeft, 0, highestValue * 1.05);
    }

    barBreakdownPlot_->replot();
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
    updateIOCharts();
    updateBreakdownCharts();

    if (model_->queryCount() <= 1)
    {
        pieStack_->setCurrentIndex(0);
        barStack_->setCurrentIndex(0);
    }
    else
    {
        pieStack_->setCurrentIndex(1);
        barStack_->setCurrentIndex(1);
    }
}

