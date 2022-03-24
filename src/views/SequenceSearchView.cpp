#include "ui_SequenceSearchView.h"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/GraphBuilder.hpp"

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(GraphBuilder* builder, QWidget* parent)
    : QWidget(parent)
    , builder_(builder)
    , ui_(new Ui::SequenceSearchView)  // Instantiate UI created in QtDesigner
{
    // Set up UI created in QtDesigner
    ui_->setupUi(this);

    builder_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
SequenceSearchView::~SequenceSearchView()
{
    // Remove things in reverse order
    builder_->dispatcher.removeListener(this);
    delete ui_;
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onGraphBuilderNewStats()
{
    ui_->label_frames->setText("Frames: " + QString::number(builder_->numFrames()));
    ui_->label_nodes->setText("Nodes: " + QString::number(builder_->numNodes()));
    ui_->label_edges->setText("Edges: " + QString::number(builder_->numEdges()));
}
