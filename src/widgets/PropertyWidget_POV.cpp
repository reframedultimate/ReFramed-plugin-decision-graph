#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/widgets/PropertyWidget_POV.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>

// ----------------------------------------------------------------------------
PropertyWidget_POV::PropertyWidget_POV(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
    , comboBox_you(new QComboBox)
    , comboBox_opp(new QComboBox)
{
    setTitle("Point of view");

    QLabel* label_you = new QLabel("You:");
    label_you->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
    QLabel* label_opp = new QLabel("Opponent:");
    label_opp->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);

    comboBox_you->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);
    comboBox_opp->setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::Preferred);

    QGridLayout* l = new QGridLayout;
    l->addWidget(label_you, 0, 0);
    l->addWidget(label_opp, 1, 0);
    l->addWidget(comboBox_you, 0, 1);
    l->addWidget(comboBox_opp, 1, 1);
    contentWidget()->setLayout(l);
    updateSize();

    connect(comboBox_you, qOverload<int>(&QComboBox::currentIndexChanged), [this] { onComboBoxPlayersChanged(); });
    connect(comboBox_you, qOverload<int>(&QComboBox::currentIndexChanged), [this] { onComboBoxPlayersChanged(); });

    seqSearchModel_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
PropertyWidget_POV::~PropertyWidget_POV()
{
    seqSearchModel_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void PropertyWidget_POV::onComboBoxPlayersChanged()
{
    seqSearchModel_->setPlayerPOV(comboBox_you->currentIndex());

    if (seqSearchModel_->applyAllQueries())
        seqSearchModel_->notifyQueriesApplied();
}

// ----------------------------------------------------------------------------
void PropertyWidget_POV::onNewSessions()
{
    QSignalBlocker blockPlayer(comboBox_you);
    QSignalBlocker blockOpponent(comboBox_opp);

    // Update dropdowns with list of players
    comboBox_you->clear();
    comboBox_opp->clear();
    for (int i = 0; i != seqSearchModel_->fighterCount(); ++i)
    {
        QString name = QString::fromUtf8(seqSearchModel_->playerName(i).cStr());
        QString fighter = QString::fromUtf8(seqSearchModel_->fighterName(i).cStr());
        QString text = name + " (" + fighter + ")";
        comboBox_you->addItem(text);
        comboBox_opp->addItem(text);
    }

    if (seqSearchModel_->fighterCount() > 0)
    {
        comboBox_you->setCurrentIndex(seqSearchModel_->playerPOV());
        comboBox_opp->setCurrentIndex(seqSearchModel_->opponentPOV());
    }
}
void PropertyWidget_POV::onClearAll()
{
    QSignalBlocker blockPlayer(comboBox_you);
    QSignalBlocker blockOpponent(comboBox_opp);

    comboBox_you->clear();
    comboBox_opp->clear();
}
void PropertyWidget_POV::onDataAdded() {}
void PropertyWidget_POV::onPOVChanged()
{
    QSignalBlocker blockPlayer(comboBox_you);
    comboBox_you->setCurrentIndex(seqSearchModel_->playerPOV());
}
void PropertyWidget_POV::onQueriesChanged() {}
void PropertyWidget_POV::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void PropertyWidget_POV::onQueriesApplied() {}
