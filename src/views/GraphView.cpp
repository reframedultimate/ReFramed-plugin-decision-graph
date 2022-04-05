#include "decision-graph/views/GraphView.hpp"
#include "decision-graph/models/GraphModel.hpp"

// ----------------------------------------------------------------------------
GraphView::GraphView(GraphModel* model, QWidget* parent)
    : QGraphicsView(parent)
    , model_(model)
{
    setScene(model);
    setDragMode(ScrollHandDrag);
    setInteractive(false);
}
