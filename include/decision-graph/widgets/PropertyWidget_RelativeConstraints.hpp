#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class PropertyWidget_RelativeConstraints : public PropertyWidget
{
public:
    PropertyWidget_RelativeConstraints(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_RelativeConstraints();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:

};
