#include "ui_SequenceSearchView.h"
#include "decision-graph/views/SequenceSearchView.hpp"

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(QWidget* parent)
    : QWidget(parent)
    , ui_(new Ui::SequenceSearchView)  // Instantiate UI created in QtDesigner
{
    // Set up UI created in QtDesigner
    ui_->setupUi(this);
}

// ----------------------------------------------------------------------------
SequenceSearchView::~SequenceSearchView()
{
    // Remove things in reverse order
    delete ui_;
}
