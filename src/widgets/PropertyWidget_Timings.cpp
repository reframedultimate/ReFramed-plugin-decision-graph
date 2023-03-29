#include "decision-graph/widgets/PropertyWidget_Timings.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QToolButton>

// ----------------------------------------------------------------------------
PropertyWidget_Timings::PropertyWidget_Timings(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
{
    setTitle("Timings settings");

    QLabel* label_firstMove = new QLabel("First move:");
    label_firstMove->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QToolButton* toolButton_prevMove1 = new QToolButton;
    toolButton_prevMove1->setIcon(QIcon::fromTheme("arrow-left"));

    QComboBox* comboBox_move1 = new QComboBox;
    comboBox_move1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QToolButton* toolButton_nextMove1 = new QToolButton;
    toolButton_nextMove1->setIcon(QIcon::fromTheme("arrow-right"));

    QLabel* label_secondMove = new QLabel("Second move:");
    label_secondMove->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    QToolButton* toolButton_prevMove2 = new QToolButton;
    toolButton_prevMove2->setIcon(QIcon::fromTheme("arrow-left"));

    QComboBox* comboBox_move2 = new QComboBox;
    comboBox_move2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

    QToolButton* toolButton_nextMove2 = new QToolButton;
    toolButton_nextMove2->setIcon(QIcon::fromTheme("arrow-right"));

    QGridLayout* l = new QGridLayout;
    l->addWidget(label_firstMove, 0, 0);
    l->addWidget(toolButton_prevMove1, 0, 1);
    l->addWidget(comboBox_move1, 0, 2);
    l->addWidget(toolButton_nextMove1, 0, 3);
    l->addWidget(label_secondMove, 1, 0);
    l->addWidget(toolButton_prevMove2, 1, 1);
    l->addWidget(comboBox_move2, 1, 2);
    l->addWidget(toolButton_nextMove2, 1, 3);

    contentWidget()->setLayout(l);
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_Timings::~PropertyWidget_Timings()
{
}
