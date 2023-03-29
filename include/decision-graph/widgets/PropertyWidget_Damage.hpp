#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class PropertyWidget_Damage : public PropertyWidget
{
public:
    PropertyWidget_Damage(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_Damage();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:

};
