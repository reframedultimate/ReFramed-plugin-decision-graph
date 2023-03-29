#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class PropertyWidget_PositionConstraints : public PropertyWidget
{
public:
    PropertyWidget_PositionConstraints(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_PositionConstraints();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:

};
