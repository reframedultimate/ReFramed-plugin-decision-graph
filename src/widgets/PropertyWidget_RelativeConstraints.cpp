#include "decision-graph/widgets/PropertyWidget_RelativeConstraints.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

// ----------------------------------------------------------------------------
PropertyWidget_RelativeConstraints::PropertyWidget_RelativeConstraints(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
{
    setTitle("Relative constraints");
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_RelativeConstraints::~PropertyWidget_RelativeConstraints()
{
}
