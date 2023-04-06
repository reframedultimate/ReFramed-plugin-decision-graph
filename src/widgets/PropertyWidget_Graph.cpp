#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/widgets/PropertyWidget_Graph.hpp"

#include "rfcommon/MotionLabels.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>
#include <QGroupBox>
#include <QRadioButton>
#include <QSpinBox>

// ----------------------------------------------------------------------------
PropertyWidget_Graph::PropertyWidget_Graph(GraphModel* graphModel, SequenceSearchModel* searchModel, rfcommon::MotionLabels* labels, QWidget* parent)
    : PropertyWidget(searchModel, parent)
    , graphModel_(graphModel)
    , labels_(labels)
    , comboBox_layer(new QComboBox)
{
    setTitle("Graph settings");

    QCheckBox* checkBox_outgoingTree = new QCheckBox("Outgoing tree:");
    QCheckBox* checkBox_incomingTree = new QCheckBox("Incoming tree:");
    checkBox_outgoingTree->setChecked(graphModel_->outgoingTreeSize() > 0);
    checkBox_incomingTree->setChecked(graphModel_->incomingTreeSize() > 0);
    checkBox_outgoingTree->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    checkBox_incomingTree->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    QSpinBox* spinBox_outgoingTree = new QSpinBox;
    spinBox_outgoingTree->setRange(1, 100);
    spinBox_outgoingTree->setValue(graphModel_->outgoingTreeSize());
    spinBox_outgoingTree->setEnabled(graphModel_->outgoingTreeSize() > 0);

    QSpinBox* spinBox_incomingTree = new QSpinBox;
    spinBox_incomingTree->setRange(1, 100);
    spinBox_incomingTree->setValue(graphModel_->incomingTreeSize());
    spinBox_incomingTree->setEnabled(graphModel_->incomingTreeSize() > 0);

    QGroupBox* groupBox_graphType = new QGroupBox;
    groupBox_graphType->setTitle("Graph type");

    QGridLayout* layout_graphType = new QGridLayout;
    layout_graphType->addWidget(checkBox_outgoingTree, 0, 0);
    layout_graphType->addWidget(checkBox_incomingTree, 1, 0);
    layout_graphType->addWidget(spinBox_outgoingTree, 0, 1);
    layout_graphType->addWidget(spinBox_incomingTree, 1, 1);
    groupBox_graphType->setLayout(layout_graphType);

    updateAvailableLayersDropdown();
    comboBox_layer->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    QCheckBox* checkBox_largestIsland = new QCheckBox;
    checkBox_largestIsland->setText("Largest island");
    checkBox_largestIsland->setToolTip(
        "If a search produces a graph with isolated subgraphs, you can check\n"
        "this to only show the largest subgraph instead of all subgraphs.");
    checkBox_largestIsland->setChecked(graphModel_->useLargestIsland());

    QCheckBox* checkBox_fixEdges = new QCheckBox;
    checkBox_fixEdges->setText("Fix edge weights");
    checkBox_fixEdges->setToolTip(
        "Sometimes the edge weights don't seem to add up correctly (you'd\n"
        "expect the sum of incoming connections to equal the sum of outgoing\n"
        "connections). Checking this will extend the ranges of search results\n"
        "to include additional nodes that would normally NOT be matched by the\n"
        "query, but the edge weights should make more sense.");
    checkBox_fixEdges->setChecked(graphModel_->useLargestIsland());

    QCheckBox* checkBox_mergeQualifiers = new QCheckBox;
    checkBox_mergeQualifiers->setText("Merge qualifiers");
    checkBox_mergeQualifiers->setChecked(graphModel_->useLargestIsland());

    QRadioButton* radioButton_dontMerge = new QRadioButton("Don't merge nodes");
    QRadioButton* radioButton_mergeQuery = new QRadioButton("Merge nodes according to query string");
    QRadioButton* radioButton_mergeLabel = new QRadioButton("Merge all nodes with same label");
    radioButton_dontMerge->setChecked(graphModel_->mergeBehavior() == GraphModel::DONT_MERGE);
    radioButton_mergeQuery->setChecked(graphModel_->mergeBehavior() == GraphModel::QUERY_MERGE);
    radioButton_mergeLabel->setChecked(graphModel_->mergeBehavior() == GraphModel::LABEL_MERGE);

    QGroupBox* groupBox_mergeSettings = new QGroupBox;
    groupBox_mergeSettings->setTitle("Graph construction");

    QVBoxLayout* layout_mergeSettings = new QVBoxLayout;
    layout_mergeSettings->addWidget(checkBox_largestIsland);
    layout_mergeSettings->addWidget(checkBox_fixEdges);
    layout_mergeSettings->addWidget(checkBox_mergeQualifiers);
    layout_mergeSettings->addWidget(radioButton_dontMerge);
    layout_mergeSettings->addWidget(radioButton_mergeQuery);
    layout_mergeSettings->addWidget(radioButton_mergeLabel);
    groupBox_mergeSettings->setLayout(layout_mergeSettings);

    QCheckBox* checkBox_showQualifiers = new QCheckBox;
    checkBox_showQualifiers->setText("Show qualifiers");
    checkBox_showQualifiers->setChecked(graphModel_->showQualifiers());

    QCheckBox* checkBox_showHash40 = new QCheckBox;
    checkBox_showHash40->setText("Show hash40 values");
    checkBox_showHash40->setChecked(graphModel_->showHash40Values());

    QLabel* label_layer = new QLabel("Preferred layer for labels:");
    label_layer->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QGroupBox* groupBox_visualSettings = new QGroupBox;
    groupBox_visualSettings->setTitle("Visual settings");

    QGridLayout* layout_visualSettings = new QGridLayout;
    layout_visualSettings->addWidget(checkBox_showQualifiers, 0, 0, 1, 2);
    layout_visualSettings->addWidget(checkBox_showHash40, 1, 0, 1, 2);
    layout_visualSettings->addWidget(label_layer, 2, 0);
    layout_visualSettings->addWidget(comboBox_layer, 2, 1);
    groupBox_visualSettings->setLayout(layout_visualSettings);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(groupBox_graphType);
    l->addWidget(groupBox_mergeSettings);
    l->addWidget(groupBox_visualSettings);

    contentWidget()->setLayout(l);
    updateSize();

    connect(checkBox_outgoingTree, &QCheckBox::toggled, [this, graphModel, spinBox_outgoingTree](bool checked) {
        spinBox_outgoingTree->setEnabled(checked);
        graphModel->setOutgoingTreeSize(checked ? spinBox_outgoingTree->value() : 0);
    });
    connect(checkBox_incomingTree, &QCheckBox::toggled, [this, graphModel, spinBox_incomingTree](bool checked) {
        spinBox_incomingTree->setEnabled(checked);
        graphModel->setIncomingTreeSize(checked ? spinBox_incomingTree->value() : 0);
    });
    connect(spinBox_outgoingTree, qOverload<int>(&QSpinBox::valueChanged), [this, graphModel](int value) {
        graphModel->setOutgoingTreeSize(value);
    });
    connect(spinBox_incomingTree, qOverload<int>(&QSpinBox::valueChanged), [this, graphModel](int value) {
        graphModel->setIncomingTreeSize(value);
    });

    connect(checkBox_largestIsland, &QCheckBox::toggled, [this, graphModel](bool checked) {
        graphModel->setUseLargestIsland(checked);
    });
    connect(checkBox_fixEdges, &QCheckBox::toggled, [this, graphModel](bool checked) {
        graphModel->setFixEdges(checked);
    });
    connect(checkBox_mergeQualifiers, &QCheckBox::toggled, [this, graphModel](bool checked) {
        graphModel->setMergeQualifiers(checked);
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
    comboBox_layer->clear();
    for (int i = 0; i != graphModel_->availableLayersCount(); ++i)
        comboBox_layer->addItem(QString::fromUtf8(graphModel_->availableLayerName(i).cStr()));
}

// ----------------------------------------------------------------------------
void PropertyWidget_Graph::onGraphModelPreferredLayerChanged()
{
    QSignalBlocker block(comboBox_layer);
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
