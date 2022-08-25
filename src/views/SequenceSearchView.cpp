#include "ui_SequenceSearchView.h"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/views/DamageView.hpp"
#include "decision-graph/views/GraphView.hpp"
#include "decision-graph/views/HeatMapView.hpp"
#include "decision-graph/views/PieChartView.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/views/ShieldHealthView.hpp"
#include "decision-graph/views/StateListView.hpp"
#include "decision-graph/views/TimingsView.hpp"

#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QGridLayout>

#include <QtCharts/QChartView>
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
    }
}

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(
        SequenceSearchModel* model,
        GraphModel* graphModel,
        LabelMapper* labels,
        QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , graphModel_(graphModel)
    , ui_(new Ui::SequenceSearchView)  // Instantiate UI created in QtDesigner
{
    // Set up UI created in QtDesigner
    ui_->setupUi(this);
    ui_->tab_stateList->setLayout(new QVBoxLayout);
    ui_->tab_stateList->layout()->addWidget(new StateListView(model, labels));

    ui_->tab_pieChart->setLayout(new QVBoxLayout);
    ui_->tab_pieChart->layout()->addWidget(new PieChartView(model, labels));

    graphModel_->addEllipse(0, 0, 10, 15);
    ui_->tab_graph->setLayout(new QVBoxLayout);
    ui_->tab_graph->layout()->addWidget(new GraphView(graphModel_));

    ui_->tab_timings->setLayout(new QVBoxLayout);
    ui_->tab_timings->layout()->addWidget(new TimingsView(model, labels));

    ui_->tab_damage->setLayout(new QVBoxLayout);
    ui_->tab_damage->layout()->addWidget(new DamageView(model, labels));

    ui_->tab_shield->setLayout(new QVBoxLayout);
    ui_->tab_shield->layout()->addWidget(new ShieldHealthView(model, labels));

    ui_->tab_heatmap->setLayout(new QVBoxLayout);
    ui_->tab_heatmap->layout()->addWidget(new HeatMapView(model, labels));

    addQueryBox();

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

    QByteArray ba = text.toUtf8();
    if (model_->setQuery(index, ba.constData()))
    {
        queryBoxes_[index].parseError->setVisible(false);
        model_->applyQuery(index);
    }
    else
    {
        queryBoxes_[index].parseError->setText(model_->lastQueryError());
        queryBoxes_[index].parseError->setVisible(true);
    }
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onComboBoxPlayerChanged(int index)
{
    model_->setCurrentFighter(index);
    model_->applyAllQueries();
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
    boxLayout->addWidget(box.parseError, 1, 1, 1, 3, Qt::AlignRight);

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

    model_->addQuery();
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

    model_->removeQuery(index);
    model_->applyAllQueries();
}

// ----------------------------------------------------------------------------
void SequenceSearchView::updateQueryStats()
{
    ui_->label_frames->setText("Frames loaded: " + QString::number(model_->totalFrameCount()));
    ui_->label_sequenceLength->setText("States loaded: " + QString::number(model_->totalSequenceLength()));
    ui_->label_matches->setText("Matched sequences: " + QString::number(model_->totalMatchedSequences()));
    ui_->label_matchedStates->setText("Matched states: " + QString::number(model_->totalMatchedStates()));
    ui_->label_matchedUniqueStates->setText("Matched unique states: " + QString::number(model_->totalMatchedUniqueStates()));
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onCurrentFighterChanged()
{
    bool blocked = ui_->comboBox_player->blockSignals(true);
        ui_->comboBox_player->setCurrentIndex(model_->currentFighter());
    ui_->comboBox_player->blockSignals(blocked);
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onNewSession()
{
    bool blocked = ui_->comboBox_player->blockSignals(true);
        ui_->comboBox_player->clear();
        for (int i = 0; i != model_->fighterCount(); ++i)
            ui_->comboBox_player->addItem(QString(model_->playerName(i)) + " (" + model_->fighterName(i) + ")");
        if (model_->fighterCount() > 0)
            ui_->comboBox_player->setCurrentIndex(model_->currentFighter());
    ui_->comboBox_player->blockSignals(blocked);
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onDataAdded()
{
    updateQueryStats();
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onDataCleared()
{
    updateQueryStats();
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onQueryApplied()
{
    updateQueryStats();
}
