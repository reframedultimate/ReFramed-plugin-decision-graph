#include "decision-graph/views/GraphView.hpp"

// ----------------------------------------------------------------------------
GraphView::GraphView(QWidget* parent)
    : QGraphicsView(parent)
{
    setDragMode(ScrollHandDrag);
    setInteractive(false);
}

// ----------------------------------------------------------------------------
GraphView::~GraphView()
{
}
