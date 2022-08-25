#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class LabelMapper;
class SequenceSearchModel;

class ShieldHealthView
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit ShieldHealthView(SequenceSearchModel* model, LabelMapper* labels, QWidget* parent=nullptr);
    ~ShieldHealthView();

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
