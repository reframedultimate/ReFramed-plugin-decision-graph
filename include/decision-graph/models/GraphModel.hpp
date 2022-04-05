#pragma once

#include <QGraphicsScene>

class Graph;

class GraphModel : public QGraphicsScene
{
public:
    void setGraph(const Graph& graph);

private:
};
