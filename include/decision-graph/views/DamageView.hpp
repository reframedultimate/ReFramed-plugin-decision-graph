#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class LabelMapper;
class SequenceSearchModel;

class DamageView
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit DamageView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent=nullptr);
    ~DamageView();

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryCompiled(int queryIdx) override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    LabelMapper* labels_;
};
