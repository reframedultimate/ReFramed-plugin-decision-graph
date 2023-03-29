#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

class PropertyWidget_Timings : public PropertyWidget
{
public:
    PropertyWidget_Timings(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_Timings();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:

};
