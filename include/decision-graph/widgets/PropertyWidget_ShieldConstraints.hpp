#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class PropertyWidget_ShieldConstraints : public PropertyWidget
{
public:
    PropertyWidget_ShieldConstraints(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_ShieldConstraints();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:

};
