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
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryCompiled(int queryIdx) override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    rfcommon::MotionLabels* labels_;

    QTextEdit* textEdit_;
};
