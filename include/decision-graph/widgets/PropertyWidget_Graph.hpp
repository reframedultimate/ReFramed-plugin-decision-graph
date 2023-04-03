#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class GraphModel;
class QComboBox;

class PropertyWidget_Graph : public PropertyWidget
{
public:
    PropertyWidget_Graph(GraphModel* graphModel, SequenceSearchModel* searchModel, QWidget* parent=nullptr);
    ~PropertyWidget_Graph();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:
    GraphModel* graphModel_;
    QComboBox* comboBox_layer;
};
