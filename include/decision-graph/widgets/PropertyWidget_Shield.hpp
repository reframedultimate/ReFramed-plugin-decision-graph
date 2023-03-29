#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class PropertyWidget_Shield : public PropertyWidget
{
public:
    PropertyWidget_Shield(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_Shield();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:

};
