#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class LabelMapper;
class SequenceSearchModel;

class StateListView
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit StateListView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent=nullptr);
    ~StateListView();

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    LabelMapper* labels_;
};
