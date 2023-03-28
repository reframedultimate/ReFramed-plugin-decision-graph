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
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileDialog>
#include <QStandardPaths>
#include <QMessageBox>

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
    connect(ui_->comboBox_opponent, qOverload<int>(&QComboBox::currentIndexChanged), [this] { onComboBoxPlayersChanged(); });
    connect(ui_->toolButton_addQuery, &QToolButton::released, this, &SequenceSearchView::addQueryBox);
    connect(ui_->pushButton_saveNew, &QPushButton::released, this, &SequenceSearchView::onPushButtonSaveNewReleased);

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
    QueryBox& box = queryBoxes_[index];

    if (box.playerQuery->text().length() == 0)
    {
        box.playerStatus->setText("Query string empty.");
        box.playerStatus->setStyleSheet("QLabel {color: #FF2020}");
        box.playerStatus->setVisible(true);
        return;
    }

    seqSearchModel_->setQuery(index,
        box.playerQuery->text().toUtf8().constData(),
        box.opponentQuery->text().toUtf8().constData());
    seqSearchModel_->notifyQueriesChanged();

    if (seqSearchModel_->compileQuery(index))
        if (seqSearchModel_->applyQuery(index))
            seqSearchModel_->notifyQueriesApplied();
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
    box.playerQuery->setPlaceholderText("You");
    box.opponentQuery = new QLineEdit;
    box.opponentQuery->setPlaceholderText("Opponent");
    box.remove = new QToolButton;
    box.remove->setIcon(QIcon::fromTheme("x"));

    QGridLayout* boxLayout = new QGridLayout;
    boxLayout->addWidget(box.name, 0, 0);
    boxLayout->addWidget(box.playerQuery, 0, 1);
    boxLayout->addWidget(box.remove, 0, 2);
    boxLayout->addWidget(box.playerStatus, 1, 0, 1, 2, Qt::AlignRight);
    boxLayout->addWidget(box.opponentQuery, 2, 1);
    boxLayout->addWidget(box.opponentStatus, 3, 0, 1, 2, Qt::AlignRight);

    QVBoxLayout* groupLayout = static_cast<QVBoxLayout*>(ui_->groupBox_query->layout());
    groupLayout->addLayout(boxLayout);
    groupLayout->addWidget(ui_->toolButton_addQuery, 0, Qt::AlignLeft);

    // Update tab order to be more convenient
    QWidget::setTabOrder(queryBoxes_[0].playerQuery, queryBoxes_[0].opponentQuery);
    for (int i = 1; i < queryBoxes_.count(); ++i)
    {
        QWidget::setTabOrder(queryBoxes_[i-1].opponentQuery, queryBoxes_[i].playerQuery);
        QWidget::setTabOrder(queryBoxes_[i].playerQuery, queryBoxes_[i].opponentQuery);
    }
    QWidget::setTabOrder(queryBoxes_.back().opponentQuery, ui_->toolButton_addQuery);
    for (int i = 1; i < queryBoxes_.count(); ++i)
        QWidget::setTabOrder(queryBoxes_[i-1].remove, queryBoxes_[i].remove);

    // Focus player query
    box.playerQuery->setFocus();

    // Because the position of the query box might change in the UI later,
    // we use one of the contained widgets to search for the current index of
    // the box. Could be any widget, we choose the name.
    QLabel* nameWidget = box.name;
    auto findBoxIndex = [this, nameWidget]() -> int {
        for (int i = 0; i != queryBoxes_.count(); ++i)
            if (queryBoxes_[i].name == nameWidget)
                return i;
        assert(false);
        return -1;
    };

    connect(box.playerQuery, &QLineEdit::textChanged, [this, findBoxIndex](const QString& text) {
        onLineEditQueryTextChanged(findBoxIndex(), text);
    });
    connect(box.opponentQuery, &QLineEdit::textChanged, [this, findBoxIndex](const QString& text) {
        onLineEditQueryTextChanged(findBoxIndex(), text);
    });
    connect(box.remove, &QToolButton::released, [this, findBoxIndex] {
        int index = findBoxIndex();

        // Remove boxlayout from outer layout and delete everything
        QLayoutItem* boxLayout = ui_->groupBox_query->layout()->takeAt(index);
        clearLayout(boxLayout->layout());
        delete boxLayout;

        // Remove widgets from array and remove query from search model
        queryBoxes_.erase(index);
        seqSearchModel_->removeQuery(index);

        // Adjust query names
        for (; index != queryBoxes_.count(); ++index)
            queryBoxes_[index].name->setText("#" + QString::number(index + 1) + ":");

        // Don't have to recompile queries, but do have to apply all again
        if (seqSearchModel_->applyAllQueries())
            seqSearchModel_->notifyQueriesApplied();
    });

    seqSearchModel_->addQuery();
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onPushButtonSaveNewReleased()
{
    QDir dir = getTemplateDir();
    QString templateName = findUnusedTemplateName(dir);
    if (saveToTemplate(dir.absoluteFilePath(templateName + ".json")) == false)
        return;

    QListWidgetItem* item = new QListWidgetItem(templateName);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    ui_->listWidget_templates->addItem(item);
    ui_->listWidget_templates->setCurrentItem(item);
    ui_->listWidget_templates->editItem(item);
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onComboBoxPlayersChanged()
{
    seqSearchModel_->setPlayerPOV(ui_->comboBox_player->currentIndex());

    if (seqSearchModel_->applyAllQueries())
        seqSearchModel_->notifyQueriesApplied();
}

// ----------------------------------------------------------------------------
QDir SequenceSearchView::getTemplateDir()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    if (dir.exists("decision-graph") == false)
        dir.mkdir("decision-graph");
    dir.cd("decision-graph");
    return dir;
}

// ----------------------------------------------------------------------------
QString SequenceSearchView::findUnusedTemplateName(const QDir& dir)
{
    int number = 1;
    while (dir.exists("Template " + QString::number(number) + ".json"))
        number++;
    return "Template " + QString::number(number);
}

// ----------------------------------------------------------------------------
bool SequenceSearchView::saveToTemplate(const QString& filePath)
{
    QJsonArray jQueries;
    for (int queryIdx = 0; queryIdx != seqSearchModel_->queryCount(); ++queryIdx)
        jQueries.push_back(QJsonObject({
            {"player", seqSearchModel_->playerQuery(queryIdx).cStr()},
            {"opponent", seqSearchModel_->opponentQuery(queryIdx).cStr()}
        }));

    QJsonObject jConstraints
    {
    };

    QJsonObject jTimingSettings
    {
        {"first", 0},
        {"second", 0}
    };

    QJsonObject jDamageSettings
    {
        {"bucketsize", 20}
    };

    QJsonObject jRoot
    {
        {"queries", jQueries},
        {"constraints", jConstraints},
        {"timings", jTimingSettings},
        {"damage", jDamageSettings}
    };

    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly) == false)
    {
        QMessageBox::critical(this, "Error", "Failed to create file \"" + filePath + "\": " + f.errorString());
        return false;
    }

    f.write(QJsonDocument(jRoot).toJson());
    return true;
}

// ----------------------------------------------------------------------------
bool SequenceSearchView::loadTemplate(const QString& filePath)
{
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly) == false)
    {
        QMessageBox::critical(this, "Error", "Failed to open file \"" + filePath + "\": " + f.errorString());
        return false;
    }

    QJsonObject jRoot = QJsonDocument::fromJson(f.readAll()).object();


    return true;
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
void SequenceSearchView::onClearAll()
{
    QSignalBlocker blockPlayer(ui_->comboBox_player);
    QSignalBlocker blockOpponent(ui_->comboBox_opponent);

    ui_->comboBox_player->clear();
    ui_->comboBox_opponent->clear();
}
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

    queryBoxes_[queryIdx].opponentStatus->setVisible(!oppSuccess);
    queryBoxes_[queryIdx].opponentStatus->setText(QString::fromUtf8(oppError));
}
void SequenceSearchView::onQueriesApplied()
{

}
