#include "decision-graph/widgets/PropertyWidget_PositionConstraints.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

// ----------------------------------------------------------------------------
PropertyWidget_PositionConstraints::PropertyWidget_PositionConstraints(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
{
    setTitle("Position constraints");
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_PositionConstraints::~PropertyWidget_PositionConstraints()
{
}
