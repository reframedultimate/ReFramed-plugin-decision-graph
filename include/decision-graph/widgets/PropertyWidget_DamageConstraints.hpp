#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class PropertyWidget_DamageConstraints : public PropertyWidget
{
public:
    PropertyWidget_DamageConstraints(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_DamageConstraints();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:

};
