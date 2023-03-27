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
            delete item;
        if (item->widget())
        {
            delete item->widget();
            delete item;
        }
    }
}

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(
        SequenceSearchModel* model,
        GraphModel* graphModel,
        rfcommon::MotionLabels* labels,
        QWidget* parent)
    : QWidget(parent)
    , seqSearchModel_(model)
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
    onTabIndexChanged(ui_->tabWidget->currentIndex());

    connect(ui_->tabWidget, &QTabWidget::currentChanged, this, &SequenceSearchView::onTabIndexChanged);
    connect(ui_->comboBox_player, qOverload<int>(&QComboBox::currentIndexChanged), [this] { onComboBoxPlayersChanged(); });
    connect(ui_->toolButton_addQuery, &QToolButton::released, this, &SequenceSearchView::addQueryBox);

    seqSearchModel_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
SequenceSearchView::~SequenceSearchView()
{
    // Remove things in reverse order
    seqSearchModel_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onTabIndexChanged(int index)
{
    switch (index)
    {
    case 0:
    case 1:
    case 2:
        ui_->groupBox_timingSettings->setVisible(false);
        ui_->groupBox_damageSettings->setVisible(false);
        ui_->groupBox_shieldSettings->setVisible(false);
        ui_->groupBox_heatMapSettings->setVisible(false);

        ui_->groupBox_damageConstraints->setVisible(true);
        ui_->groupBox_shieldConstraints->setVisible(true);
        ui_->groupBox_posConstraints->setVisible(true);
        ui_->groupBox_relPosConstraints->setVisible(true);
        break;
    case 3:
        ui_->groupBox_timingSettings->setVisible(true);
        ui_->groupBox_damageSettings->setVisible(false);
        ui_->groupBox_shieldSettings->setVisible(false);
        ui_->groupBox_heatMapSettings->setVisible(false);

        ui_->groupBox_damageConstraints->setVisible(true);
        ui_->groupBox_shieldConstraints->setVisible(true);
        ui_->groupBox_posConstraints->setVisible(true);
        ui_->groupBox_relPosConstraints->setVisible(true);
        break;
    case 4:
        ui_->groupBox_timingSettings->setVisible(false);
        ui_->groupBox_damageSettings->setVisible(true);
        ui_->groupBox_shieldSettings->setVisible(false);
        ui_->groupBox_heatMapSettings->setVisible(false);

        ui_->groupBox_damageConstraints->setVisible(false);
        ui_->groupBox_shieldConstraints->setVisible(true);
        ui_->groupBox_posConstraints->setVisible(true);
        ui_->groupBox_relPosConstraints->setVisible(true);
        break;
    case 5:
        ui_->groupBox_timingSettings->setVisible(false);
        ui_->groupBox_damageSettings->setVisible(false);
        ui_->groupBox_shieldSettings->setVisible(true);
        ui_->groupBox_heatMapSettings->setVisible(false);

        ui_->groupBox_damageConstraints->setVisible(true);
        ui_->groupBox_shieldConstraints->setVisible(false);
        ui_->groupBox_posConstraints->setVisible(true);
        ui_->groupBox_relPosConstraints->setVisible(true);
        break;

    case 6:
    case 7:
        ui_->groupBox_timingSettings->setVisible(false);
        ui_->groupBox_damageSettings->setVisible(false);
        ui_->groupBox_shieldSettings->setVisible(false);
        ui_->groupBox_heatMapSettings->setVisible(true);

        ui_->groupBox_damageConstraints->setVisible(true);
        ui_->groupBox_shieldConstraints->setVisible(true);
        ui_->groupBox_posConstraints->setVisible(true);
        ui_->groupBox_relPosConstraints->setVisible(true);
        break;
    }
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onLineEditQueryTextChanged(int index, const QString& text)
{
    if (text.length() == 0)
    {
        queryBoxes_[index].playerStatus->setText("Query string empty.");
        queryBoxes_[index].playerStatus->setStyleSheet("QLabel {color: #FF2020}");
        queryBoxes_[index].playerStatus->setVisible(true);
        return;
    }

    seqSearchModel_->setQuery(index,
        queryBoxes_[index].playerQuery->text().toUtf8().constData(),
        queryBoxes_[index].opponentQuery->text().toUtf8().constData());
    seqSearchModel_->notifyQueriesChanged();

    if (seqSearchModel_->playerPOV() >= 0 && seqSearchModel_->opponentPOV() >= 0)
    {
        seqSearchModel_->compileQuery(index,
            seqSearchModel_->fighterID(seqSearchModel_->playerPOV()),
            seqSearchModel_->fighterID(seqSearchModel_->opponentPOV()));

        if (seqSearchModel_->applyQuery(index,
                seqSearchModel_->fighterStates(seqSearchModel_->playerPOV()),
                seqSearchModel_->fighterStates(seqSearchModel_->opponentPOV())))
        {
            seqSearchModel_->notifyQueriesApplied();
        }
    }
}

// ----------------------------------------------------------------------------
void SequenceSearchView::addQueryBox()
{
    ui_->toolButton_addQuery->setParent(nullptr);

    QueryBox& box = queryBoxes_.emplace();
    box.name = new QLabel("#" + QString::number(queryBoxes_.count()) + ":");
    box.playerStatus = new QLabel("Query string empty.");
    box.playerStatus->setStyleSheet("QLabel {color: #FF2020}");
    box.opponentStatus = new QLabel;
    box.opponentStatus->setVisible(false);
    box.playerQuery = new QLineEdit;
    box.opponentQuery = new QLineEdit;
    box.opponentQuery->setPlaceholderText("Opponent search string");
    box.remove = new QToolButton;
    box.remove->setIcon(QIcon::fromTheme("x"));

    QGridLayout* boxLayout = new QGridLayout;
    boxLayout->addWidget(box.name, 0, 0);
    boxLayout->addWidget(box.playerQuery, 0, 1);
    boxLayout->addWidget(box.remove, 0, 2);
    boxLayout->addWidget(box.playerStatus, 1, 1, 1, 3, Qt::AlignRight);
    boxLayout->addWidget(box.opponentQuery, 2, 1);
    boxLayout->addWidget(box.opponentStatus, 3, 1, 1, 3, Qt::AlignRight);

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

    QLineEdit* queryEdit = box.playerQuery;
    connect(box.playerQuery, &QLineEdit::textChanged, [this, queryEdit](const QString& text) {
        for (int i = 0; i != queryBoxes_.count(); ++i)
            if (queryBoxes_[i].playerQuery == queryEdit)
            {
                onLineEditQueryTextChanged(i, text);
                break;
            }
    });

    seqSearchModel_->addQuery();
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

    seqSearchModel_->removeQuery(index);
    seqSearchModel_->applyAllQueries(
        seqSearchModel_->fighterStates(seqSearchModel_->playerPOV()),
        seqSearchModel_->fighterStates(seqSearchModel_->opponentPOV())
    );
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onComboBoxPlayersChanged()
{
    seqSearchModel_->setPlayerPOV(ui_->comboBox_player->currentIndex());

    seqSearchModel_->applyAllQueries(
         seqSearchModel_->fighterStates(seqSearchModel_->playerPOV()),
         seqSearchModel_->fighterStates(seqSearchModel_->opponentPOV()));
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onNewSessions()
{
    // Update dropdowns with list of players
    QSignalBlocker blockPlayer(ui_->comboBox_player);
    QSignalBlocker blockOpponent(ui_->comboBox_opponent);

    ui_->comboBox_player->clear();
    ui_->comboBox_opponent->clear();
    for (int i = 0; i != seqSearchModel_->fighterCount(); ++i)
    {
        QString name = QString::fromUtf8(seqSearchModel_->playerName(i).cStr());
        QString fighter = QString::fromUtf8(seqSearchModel_->fighterName(i).cStr());
        QString text = name + " (" + fighter + ")";
        ui_->comboBox_player->addItem(text);
        ui_->comboBox_opponent->addItem(text);
    }

    if (seqSearchModel_->fighterCount() > 0)
    {
        ui_->comboBox_player->setCurrentIndex(seqSearchModel_->playerPOV());
        ui_->comboBox_opponent->setCurrentIndex(seqSearchModel_->opponentPOV());
    }
}
void SequenceSearchView::onClearAll() {}
void SequenceSearchView::onDataAdded() {}
void SequenceSearchView::onPOVChanged()
{
    QSignalBlocker blockPlayer(ui_->comboBox_player);
    ui_->comboBox_player->setCurrentIndex(seqSearchModel_->playerPOV());
}
void SequenceSearchView::onQueriesChanged() {}
void SequenceSearchView::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError)
{
    queryBoxes_[queryIdx].playerStatus->setVisible(!success);
    queryBoxes_[queryIdx].playerStatus->setText(QString::fromUtf8(error));
}
void SequenceSearchView::onQueriesApplied()
{

}
