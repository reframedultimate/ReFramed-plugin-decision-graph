#include "ui_SequenceSearchView.h"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/views/GraphView.hpp"

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(SequenceSearchModel* model, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , graphModel_(new GraphModel)
    , ui_(new Ui::SequenceSearchView)  // Instantiate UI created in QtDesigner
{
    // Set up UI created in QtDesigner
    ui_->setupUi(this);
    ui_->label_parseError->setText("Query string empty.");
    ui_->label_parseError->setStyleSheet("QLabel {color: #FF2020}");

    graphModel_->addEllipse(0, 0, 10, 15);
    ui_->tab_graph->setLayout(new QVBoxLayout);
    ui_->tab_graph->layout()->addWidget(new GraphView(graphModel_.get()));

    SequenceSearchView::onSessionChanged();

    connect(ui_->lineEdit_query, &QLineEdit::textChanged,
            this, &SequenceSearchView::onLineEditQueryTextChanged);
    connect(ui_->comboBox_player, qOverload<int>(&QComboBox::currentIndexChanged),
            this, &SequenceSearchView::onComboBoxPlayerChanged);

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
void SequenceSearchView::onLineEditQueryTextChanged(const QString& text)
{
    if (text.length() == 0)
    {
        ui_->label_parseError->setText("Query string empty.");
        ui_->label_parseError->setStyleSheet("QLabel {color: #FF2020}");
        return;
    }

    QByteArray ba = text.toLocal8Bit();
    if (model_->setQuery(ba.data()))
    {
        ui_->label_parseError->setText("Query string Valid.");
        ui_->label_parseError->setStyleSheet("QLabel {color: #20C020}");
    }
    else
    {
        ui_->label_parseError->setText(model_->queryError());
        ui_->label_parseError->setStyleSheet("QLabel {color: #FF2020}");
    }
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onComboBoxPlayerChanged(int index)
{
    model_->setCurrentFighter(index);
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

    ui_->listWidget_labels->clear();
    for (const auto& label : model_->availableLabels(model_->currentFighter()))
        ui_->listWidget_labels->addItem(label.cStr());

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

    ui_->listWidget_labels->clear();
    for (const auto& label : model_->availableLabels(model_->currentFighter()))
        ui_->listWidget_labels->addItem(label.cStr());

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
