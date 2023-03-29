#include "decision-graph/widgets/PropertyWidget_HeatMap.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

// ----------------------------------------------------------------------------
PropertyWidget_HeatMap::PropertyWidget_HeatMap(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
{
    setTitle("Heat map settings");
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_HeatMap::~PropertyWidget_HeatMap()
{
}
