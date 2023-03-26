#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>

class SequenceSearchModel;

namespace rfcommon {
    class MotionLabels;
}

class ShieldHealthView
        : public QWidget
        , public SequenceSearchListener
{
public:
    explicit ShieldHealthView(SequenceSearchModel* model, rfcommon::MotionLabels* labels, QWidget* parent=nullptr);
    ~ShieldHealthView();

private:
    void onPOVChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryCompiled(int queryIdx) override;
    void onQueryApplied() override;

private:
    SequenceSearchModel* model_;
    rfcommon::MotionLabels* labels_;
};
