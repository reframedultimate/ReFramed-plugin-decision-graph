#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class PropertyWidget_HeatMap : public PropertyWidget
{
public:
    PropertyWidget_HeatMap(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_HeatMap();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:

};
