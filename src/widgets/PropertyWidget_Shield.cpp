#include "decision-graph/widgets/PropertyWidget_Shield.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

// ----------------------------------------------------------------------------
PropertyWidget_Shield::PropertyWidget_Shield(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
{
    setTitle("Shield settings");
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_Shield::~PropertyWidget_Shield()
{
}
