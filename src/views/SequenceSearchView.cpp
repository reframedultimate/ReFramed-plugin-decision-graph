#include "ui_SequenceSearchView.h"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/views/GraphView.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"

#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QGridLayout>

#include <QtCharts/QChartView>
#include <QtCharts/QPieSeries>
#include <QtCharts/QPieSlice>

#include <QtWidgets/QApplication>
#include <QtWidgets/QMainWindow>
#include <QtCharts/QChartView>
#include <QtCharts/QStackedBarSeries>
#include <QtCharts/QBarSet>
#include <QtCharts/QLegend>
#include <QtCharts/QBarCategoryAxis>
#include <QtCharts/QValueAxis>

QT_CHARTS_USE_NAMESPACE

// ----------------------------------------------------------------------------
static void clearLayout(QLayout* layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->layout())
            delete item->layout();
        if (item->widget())
            delete item->widget();
        delete item;
    }
}

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(
        SequenceSearchModel* model,
        GraphModel* graphModel,
        QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , graphModel_(graphModel)
    , ui_(new Ui::SequenceSearchView)  // Instantiate UI created in QtDesigner
{
    // Set up UI created in QtDesigner
    ui_->setupUi(this);

    addQueryBox();

    graphModel_->addEllipse(0, 0, 10, 15);
    ui_->tab_graph->setLayout(new QVBoxLayout);
    ui_->tab_graph->layout()->addWidget(new GraphView(graphModel_));

    {
        QPieSeries* series = new QPieSeries();
        series->append("Jane", 1);
        series->append("Joe", 2);
        series->append("Andy", 3);
        series->append("Barbara", 4);
        series->append("Axel", 5);

        QPieSlice* slice = series->slices().at(1);
        slice->setExploded();
        slice->setLabelVisible();
        slice->setPen(QPen(Qt::darkGreen, 2));
        slice->setBrush(Qt::green);

        QChart* chart = new QChart();
        chart->addSeries(series);
        chart->setTitle("Simple piechart example");
        chart->legend()->hide();

        QChartView* chartView = new QChartView(chart);
        chartView->setRenderHint(QPainter::Antialiasing);

        ui_->tab_pieChart->setLayout(new QVBoxLayout);
        ui_->tab_pieChart->layout()->addWidget(chartView);
    }

    {
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
        ui_->tab_stateList->layout()->addWidget(chartView);
    }

    SequenceSearchView::onSessionChanged();

    connect(ui_->comboBox_player, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &SequenceSearchView::onComboBoxPlayerChanged);
    connect(ui_->toolButton_addQuery, &QToolButton::released, 
            this, &SequenceSearchView::addQueryBox);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
SequenceSearchView::~SequenceSearchView()
{
    // Remove things in reverse order
    model_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onLineEditQueryTextChanged(int index, const QString& text)
{
    if (text.length() == 0)
    {
        queryBoxes_[index].parseError->setText("Query string empty.");
        queryBoxes_[index].parseError->setVisible(true);
        return;
    }

    QByteArray ba = text.toLocal8Bit();
    if (model_->setQuery(ba.data(), model_->currentFighter()))
    {
        queryBoxes_[index].parseError->setVisible(false);
    }
    else
    {
        queryBoxes_[index].parseError->setText(model_->queryError());
        queryBoxes_[index].parseError->setVisible(true);
    }
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onComboBoxPlayerChanged(int index)
{
    model_->setCurrentFighter(index);
}

// ----------------------------------------------------------------------------
void SequenceSearchView::addQueryBox()
{
    ui_->toolButton_addQuery->setParent(nullptr);

    QueryBox& box = queryBoxes_.emplace();
    box.name = new QLabel("#" + QString::number(queryBoxes_.count()) + ":");
    box.parseError = new QLabel("Query text empty.");
    box.parseError->setStyleSheet("QLabel {color: #FF2020}");
    box.query = new QLineEdit;
    box.remove = new QToolButton;
    box.remove->setText("X");

    QGridLayout* boxLayout = new QGridLayout;
    boxLayout->addWidget(box.name, 0, 0);
    boxLayout->addWidget(box.query, 0, 1);
    boxLayout->addWidget(box.remove, 0, 2);
    boxLayout->addWidget(box.parseError, 1, 1, 1, 3, Qt::AlignLeft);

    QVBoxLayout* groupLayout = static_cast<QVBoxLayout*>(ui_->groupBox_query->layout());
    groupLayout->addLayout(boxLayout);
    groupLayout->addWidget(ui_->toolButton_addQuery, 0, Qt::AlignLeft);

    QToolButton* removeButton = box.remove;
    connect(box.remove, &QToolButton::released, [this, removeButton] {
        for (int i = 0; i != queryBoxes_.count(); ++i)
            if (queryBoxes_[i].remove == removeButton)
            {
                removeQueryBox(i);
                break;
            }
    });

    QLineEdit* queryEdit = box.query;
    connect(box.query, &QLineEdit::textChanged, [this, queryEdit](const QString& text) {
        for (int i = 0; i != queryBoxes_.count(); ++i)
            if (queryBoxes_[i].query == queryEdit)
            {
                onLineEditQueryTextChanged(i, text);
                break;
            }
    });
}

// ----------------------------------------------------------------------------
void SequenceSearchView::removeQueryBox(int index)
{
    auto boxLayout = ui_->groupBox_query->layout()->takeAt(index);
    clearLayout(boxLayout->layout());
    delete boxLayout;

    queryBoxes_.erase(index);

    for (; index != queryBoxes_.count(); ++index)
        queryBoxes_[index].name->setText("#" + QString::number(index + 1) + ":");
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onSessionChanged()
{
    bool blocked = ui_->comboBox_player->blockSignals(true);

    ui_->comboBox_player->clear();
    for (int i = 0; i != model_->fighterCount(); ++i)
        ui_->comboBox_player->addItem(QString(model_->fighterName(i)) + " (" + model_->fighterCharacter(i) + ")");
    if (model_->fighterCount() > 0)
        ui_->comboBox_player->setCurrentIndex(model_->currentFighter());
    ui_->comboBox_player->blockSignals(blocked);

    /*
    ui_->listWidget_labels->clear();
    for (const auto& label : model_->availableLabels(model_->currentFighter()))
        ui_->listWidget_labels->addItem(label.cStr());*/

    ui_->label_frames->setText("Total Frames: " + QString::number(model_->frameCount()));
    ui_->label_sequenceLength->setText(
                "Total Sequence Length: " +
                QString::number(model_->sequenceLength(model_->currentFighter())));

    int numMatches, numMatchedStates;
    Graph graph = model_->applyQuery(&numMatches, &numMatchedStates);
    ui_->label_matches->setText("Matches: " + QString::number(numMatches));
    ui_->label_matchedStates->setText("Matched States: " + QString::number(numMatchedStates));
    ui_->label_matchedUniqueStates->setText("Matched Unique States: " + QString::number(graph.nodes.count()));
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onCurrentFighterChanged()
{
    bool blocked = ui_->comboBox_player->blockSignals(true);
    ui_->comboBox_player->setCurrentIndex(model_->currentFighter());
    ui_->comboBox_player->blockSignals(blocked);

    /*
    ui_->listWidget_labels->clear();
    for (const auto& label : model_->availableLabels(model_->currentFighter()))
        ui_->listWidget_labels->addItem(label.cStr());*/

    int numMatches, numMatchedStates;
    Graph graph = model_->applyQuery(&numMatches, &numMatchedStates);
    ui_->label_matches->setText("Matches: " + QString::number(numMatches));
    ui_->label_matchedStates->setText("Matched States: " + QString::number(numMatchedStates));
    ui_->label_matchedUniqueStates->setText("Matched Unique States: " + QString::number(graph.nodes.count()));
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onSequenceChanged()
{
    ui_->label_sequenceLength->setText(
                "Total Sequence Length: " +
                QString::number(model_->sequenceLength(model_->currentFighter())));

    int numMatches, numMatchedStates;
    Graph graph = model_->applyQuery(&numMatches, &numMatchedStates);
    ui_->label_matches->setText("Matches: " + QString::number(numMatches));
    ui_->label_matchedStates->setText("Matched States: " + QString::number(numMatchedStates));
    ui_->label_matchedUniqueStates->setText("Matched Unique States: " + QString::number(graph.nodes.count()));
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onQueryChanged()
{
    int numMatches, numMatchedStates;
    Graph graph = model_->applyQuery(&numMatches, &numMatchedStates);
    ui_->label_matches->setText("Matches: " + QString::number(numMatches));
    ui_->label_matchedStates->setText("Matched States: " + QString::number(numMatchedStates));
    ui_->label_matchedUniqueStates->setText("Matched Unique States: " + QString::number(graph.nodes.count()));
}
