#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

namespace rfcommon {
    class MotionLabels;
}

class SequenceSearchModel;
class QTextEdit;

class StateListView
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit StateListView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent=nullptr);
    ~StateListView();

private:
    void updateText();

private:
    void onNewSessions() override;
    void onClearAll() override;
    void onDataAdded() override;
    void onPOVChanged() override;
    void onQueriesChanged() override;
    void onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) override;
    void onQueriesApplied() override;

private:
    SequenceSearchModel* model_;
    rfcommon::MotionLabels* labels_;

    QTextEdit* textEdit_;
};
