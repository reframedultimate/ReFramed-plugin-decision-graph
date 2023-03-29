#include "decision-graph/widgets/PropertyWidget_Damage.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QSpinBox>
#include <QToolButton>

// ----------------------------------------------------------------------------
PropertyWidget_Damage::PropertyWidget_Damage(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
{
    setTitle("Damage settings");

    QSpinBox* spinBox_bucketSize = new QSpinBox;
    spinBox_bucketSize->setMinimum(5);
    spinBox_bucketSize->setMaximum(100);
    spinBox_bucketSize->setSingleStep(5);
    spinBox_bucketSize->setValue(20);
    spinBox_bucketSize->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QLabel* label_bucketSize = new QLabel("Bucket size:");
    label_bucketSize->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QLabel* label_percent = new QLabel("%");
    label_percent->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QGridLayout* l = new QGridLayout;
    l->addWidget(label_bucketSize, 0, 0);
    l->addWidget(spinBox_bucketSize, 0, 2);
    l->addWidget(label_percent, 0, 3);

    contentWidget()->setLayout(l);
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_Damage::~PropertyWidget_Damage()
{
}
