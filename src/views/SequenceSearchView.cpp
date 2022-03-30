#include "ui_SequenceSearchView.h"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/IncrementalData.hpp"

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(IncrementalData* builder, QWidget* parent)
    : QWidget(parent)
    , incData_(builder)
    , ui_(new Ui::SequenceSearchView)  // Instantiate UI created in QtDesigner
{
    // Set up UI created in QtDesigner
    ui_->setupUi(this);

    incData_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
SequenceSearchView::~SequenceSearchView()
{
    // Remove things in reverse order
    incData_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onIncrementalDataNewStats()
{
    ui_->label_frames->setText("Frames: " + QString::number(incData_->numFrames()));
    ui_->label_states->setText("States: " + QString::number(incData_->sequence(0).states.count()));
    ui_->label_nodes->setText("Nodes: " + QString::number(incData_->graph(0).nodes.count()));
    ui_->label_edges->setText("Edges: " + QString::number(incData_->graph(0).edges.count()));
}
