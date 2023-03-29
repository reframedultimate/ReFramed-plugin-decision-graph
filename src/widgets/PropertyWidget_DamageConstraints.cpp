#include "decision-graph/widgets/PropertyWidget_DamageConstraints.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

// ----------------------------------------------------------------------------
PropertyWidget_DamageConstraints::PropertyWidget_DamageConstraints(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
{
    setTitle("Damage constraints");
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_DamageConstraints::~PropertyWidget_DamageConstraints()
{
}
