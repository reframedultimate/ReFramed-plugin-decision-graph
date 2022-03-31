#include "ui_SequenceSearchView.h"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/IncrementalData.hpp"
#include "decision-graph/models/QueryBuilder.hpp"

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(IncrementalData* incData, MotionsTable* motionsTable, QWidget* parent)
    : QWidget(parent)
    , incData_(incData)
    , motionsTable_(motionsTable)
    , ui_(new Ui::SequenceSearchView)  // Instantiate UI created in QtDesigner
{
    // Set up UI created in QtDesigner
    ui_->setupUi(this);

    connect(ui_->lineEdit_query, &QLineEdit::textChanged,
            this, &SequenceSearchView::onLineEditQueryTextChanged);

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
void SequenceSearchView::onLineEditQueryTextChanged(const QString& text)
{
    QByteArray ba = text.toLocal8Bit();
    QueryBuilder builder(motionsTable_);
    if (builder.parse(ba.data()))
    {

    }
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onIncrementalDataNewStats()
{
}
