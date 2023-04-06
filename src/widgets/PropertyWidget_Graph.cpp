#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/widgets/PropertyWidget_Graph.hpp"

#include "rfcommon/MotionLabels.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>

// ----------------------------------------------------------------------------
PropertyWidget_Graph::PropertyWidget_Graph(GraphModel* graphModel, SequenceSearchModel* searchModel, rfcommon::MotionLabels* labels, QWidget* parent)
    : PropertyWidget(searchModel, parent)
    , graphModel_(graphModel)
    , labels_(labels)
    , comboBox_layer(new QComboBox)
{
    setTitle("Graph settings");

    QRadioButton* radioButton_fullGraph = new QRadioButton("Full graph");
    QRadioButton* radioButton_outgoingTree = new QRadioButton("Outgoing tree");
    QRadioButton* radioButton_incomingTree = new QRadioButton("Incoming tree");
    radioButton_fullGraph->setChecked(graphModel_->graphType() == GraphModel::FULL_GRAPH);
    radioButton_outgoingTree->setChecked(graphModel_->graphType() == GraphModel::OUTGOING_TREE);
    radioButton_incomingTree->setChecked(graphModel_->graphType() == GraphModel::INCOMING_TREE);

    QGroupBox* groupBox_graphType = new QGroupBox;
    groupBox_graphType->setTitle("Graph type");

    QVBoxLayout* layout_graphType = new QVBoxLayout;
    layout_graphType->addWidget(radioButton_fullGraph);
    layout_graphType->addWidget(radioButton_outgoingTree);
    layout_graphType->addWidget(radioButton_incomingTree);
    groupBox_graphType->setLayout(layout_graphType);

    updateAvailableLayersDropdown();
    comboBox_layer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    QCheckBox* checkBox_largestIsland = new QCheckBox;
    checkBox_largestIsland->setText("Only use largest island");
    checkBox_largestIsland->setChecked(graphModel_->useLargestIsland());

    QRadioButton* radioButton_dontMerge = new QRadioButton("Don't merge nodes");
    QRadioButton* radioButton_mergeQuery = new QRadioButton("Merge nodes according to query string");
    QRadioButton* radioButton_mergeLabel = new QRadioButton("Merge all nodes with same label");
    radioButton_dontMerge->setChecked(graphModel_->mergeBehavior() == GraphModel::DONT_MERGE);
    radioButton_mergeQuery->setChecked(graphModel_->mergeBehavior() == GraphModel::QUERY_MERGE);
    radioButton_mergeLabel->setChecked(graphModel_->mergeBehavior() == GraphModel::LABEL_MERGE);

    QLabel* label_layer = new QLabel("Preferred layer for labels:");
    label_layer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QGroupBox* groupBox_mergeSettings = new QGroupBox;
    groupBox_mergeSettings->setTitle("Graph construction");

    QGridLayout* layout_mergeSettings = new QGridLayout;
    layout_mergeSettings->addWidget(checkBox_largestIsland, 0, 0, 1, 2);
    layout_mergeSettings->addWidget(radioButton_dontMerge, 1, 0, 1, 2);
    layout_mergeSettings->addWidget(radioButton_mergeQuery, 2, 0, 1, 2);
    layout_mergeSettings->addWidget(radioButton_mergeLabel, 3, 0, 1, 2);
    layout_mergeSettings->addWidget(label_layer, 4, 0);
    layout_mergeSettings->addWidget(comboBox_layer, 4, 1);
    groupBox_mergeSettings->setLayout(layout_mergeSettings);

    QCheckBox* checkBox_showHash40 = new QCheckBox;
    checkBox_showHash40->setText("Show hash40 values");
    checkBox_showHash40->setChecked(graphModel_->showHash40Values());

    QCheckBox* checkBox_showQualifiers = new QCheckBox;
    checkBox_showQualifiers->setText("Show qualifiers");
    checkBox_showQualifiers->setChecked(graphModel_->showQualifiers());

    QGroupBox* groupBox_visualSettings = new QGroupBox;
    groupBox_visualSettings->setTitle("Visual settings");

    QVBoxLayout* layout_visualSettings = new QVBoxLayout;
    layout_visualSettings->addWidget(checkBox_showHash40);
    layout_visualSettings->addWidget(checkBox_showQualifiers);
    groupBox_visualSettings->setLayout(layout_visualSettings);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(groupBox_graphType);
    l->addWidget(groupBox_mergeSettings);
    l->addWidget(groupBox_visualSettings);

    contentWidget()->setLayout(l);
    updateSize();

    connect(radioButton_fullGraph, &QRadioButton::toggled, [this, graphModel](bool checked) {
        if (checked)
            graphModel->setGraphType(GraphModel::FULL_GRAPH);
    });
    connect(radioButton_outgoingTree, &QRadioButton::toggled, [this, graphModel](bool checked) {
        if (checked)
            graphModel->setGraphType(GraphModel::OUTGOING_TREE);
    });
    connect(radioButton_incomingTree, &QRadioButton::toggled, [this, graphModel](bool checked) {
        if (checked)
            graphModel->setGraphType(GraphModel::INCOMING_TREE);
    });

    connect(checkBox_largestIsland, &QCheckBox::toggled, [this, graphModel](bool checked) {
        graphModel->setUseLargestIsland(checked);
    });

    connect(radioButton_dontMerge, &QRadioButton::toggled, [this, graphModel](bool checked) {
        if (checked)
            graphModel->setMergeBehavior(GraphModel::DONT_MERGE);
    });
    connect(radioButton_mergeQuery, &QRadioButton::toggled, [this, graphModel](bool checked) {
        if (checked)
            graphModel->setMergeBehavior(GraphModel::QUERY_MERGE);
    });
    connect(radioButton_mergeLabel, &QRadioButton::toggled, [this, graphModel](bool checked) {
        if (checked)
            graphModel->setMergeBehavior(GraphModel::LABEL_MERGE);
    });

    connect(comboBox_layer, qOverload<int>(&QComboBox::currentIndexChanged), [this, graphModel](int index) {
        graphModel->setPreferredLayer(index);
    });

    connect(checkBox_showHash40, &QCheckBox::toggled, [this, graphModel](bool checked) {
        graphModel->setShowHash40Values(checked);
    });
    connect(checkBox_showQualifiers, &QCheckBox::toggled, [this, graphModel](bool checked) {
        graphModel->setShowQualifiers(checked);
    });

    labels_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
PropertyWidget_Graph::~PropertyWidget_Graph()
{
    labels_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void PropertyWidget_Graph::updateAvailableLayersDropdown()
{
    QSignalBlocker block(comboBox_layer);

    comboBox_layer->clear();
    for (int i = 0; i != graphModel_->availableLayersCount(); ++i)
        comboBox_layer->addItem(QString::fromUtf8(graphModel_->availableLayerName(i).cStr()));
    comboBox_layer->setCurrentIndex(graphModel_->preferredLayer());
}

// ----------------------------------------------------------------------------
void PropertyWidget_Graph::onMotionLabelsLoaded() { updateAvailableLayersDropdown(); }
void PropertyWidget_Graph::onMotionLabelsHash40sUpdated() {}

void PropertyWidget_Graph::onMotionLabelsPreferredLayerChanged(int usage) { updateAvailableLayersDropdown(); }

void PropertyWidget_Graph::onMotionLabelsLayerInserted(int layerIdx) { updateAvailableLayersDropdown(); }
void PropertyWidget_Graph::onMotionLabelsLayerRemoved(int layerIdx) { updateAvailableLayersDropdown(); }
void PropertyWidget_Graph::onMotionLabelsLayerNameChanged(int layerIdx) { updateAvailableLayersDropdown(); }
void PropertyWidget_Graph::onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) { updateAvailableLayersDropdown(); }
void PropertyWidget_Graph::onMotionLabelsLayerMoved(int fromIdx, int toIdx) { updateAvailableLayersDropdown(); }
void PropertyWidget_Graph::onMotionLabelsLayerMerged(int layerIdx) { updateAvailableLayersDropdown(); }

void PropertyWidget_Graph::onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) {}
void PropertyWidget_Graph::onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) {}
void PropertyWidget_Graph::onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) {}
