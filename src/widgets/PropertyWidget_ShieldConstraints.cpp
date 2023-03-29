#include "decision-graph/widgets/PropertyWidget_ShieldConstraints.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

// ----------------------------------------------------------------------------
PropertyWidget_ShieldConstraints::PropertyWidget_ShieldConstraints(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
{
    setTitle("Shield constraints");
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_ShieldConstraints::~PropertyWidget_ShieldConstraints()
{
}
