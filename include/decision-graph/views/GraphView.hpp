#pragma once

#include <QGraphicsView>

class SequenceSearchModel;
class GraphModel;

class GraphView : public QGraphicsView
{
    Q_OBJECT

public:
    explicit GraphView(QWidget* parent=nullptr);
    ~GraphView();

private:
};
