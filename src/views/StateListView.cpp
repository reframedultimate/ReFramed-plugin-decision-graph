#include "decision-graph/views/StateListView.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/models/Sequence.hpp"

#include <QTextEdit>
#include <QVBoxLayout>

// ----------------------------------------------------------------------------
StateListView::StateListView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent)
    : QWidget(parent)
    , model_(model)
    , labels_(labels)
    , textEdit_(new QTextEdit)
{
    textEdit_->setReadOnly(true);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(textEdit_);
    setLayout(l);

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
void StateListView::updateText()
{
    textEdit_->clear();

    const States& states = model_->fighterStates(model_->playerPOV());

    for (int queryIdx = 0; queryIdx != model_->queryCount(); ++queryIdx)
    {
        QTextCursor cursor = textEdit_->textCursor();
        cursor.insertText("Query: " + QString::fromUtf8(model_->playerQuery(queryIdx).cStr()) + "\n");

        for (int sessionIdx = 0; sessionIdx != model_->sessionCount(); ++sessionIdx)
        {
            cursor.insertText("  Replay: " + QString::fromUtf8(model_->sessionName(sessionIdx)) + "\n");
            for (const Range& range : model_->sessionMatches(queryIdx, sessionIdx))
                cursor.insertText("    " + QString::fromUtf8(toString(states, range, labels_).cStr()) + "\n");
        }
    }
}

// ----------------------------------------------------------------------------
StateListView::~StateListView()
{
    model_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void StateListView::onNewSessions() {}
void StateListView::onClearAll() {}
void StateListView::onDataAdded() {}
void StateListView::onPOVChanged() {}
void StateListView::onQueriesChanged() {}
void StateListView::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void StateListView::onQueriesApplied() { updateText(); }
