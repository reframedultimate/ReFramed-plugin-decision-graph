#pragma once

#include <QGraphicsView>

class GraphModel;

class GraphView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphView(GraphModel* model, QWidget* parent=nullptr);

private:
    GraphModel* model_;
};
