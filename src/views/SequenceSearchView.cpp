#include "ui_SequenceSearchView.h"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/models/IncrementalData.hpp"
#include "decision-graph/models/Query.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"

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
    QueryASTNode* ast;
    Query* query;

    QByteArray ba = text.toLocal8Bit();
    ast = Query::parse(ba.data());
    if (ast == nullptr)
    {
        ui_->label_parseError->setText("Parse error");
        return;
    }
    ast->exportDOT("query-ast.dot");

    query = Query::compileAST(ast, motionsTable_);
    QueryASTNode::destroyRecurse(ast);
    if (query == nullptr)
    {
        ui_->label_parseError->setText("Compile error");
        return;
    }
    query->exportDOT("query.dot", motionsTable_);

    query_.reset(query);
    ui_->label_parseError->setText("No errors found.");

    applyCurrentQuery();
}

// ----------------------------------------------------------------------------
void SequenceSearchView::applyCurrentQuery()
{
    if (incData_->session() == nullptr || query_.get() == nullptr)
        return;

    rfcommon::Vector<SequenceRange> queryResult = query_->apply(incData_->sequence(0));
    Graph graph = Graph::fromSequenceRanges(incData_->sequence(0), queryResult);
    graph.exportDOT("decision_graph_search.dot", 0, incData_->session(), motionsTable_);
}

// ----------------------------------------------------------------------------
void SequenceSearchView::onIncrementalDataNewStats()
{
    applyCurrentQuery();
}
