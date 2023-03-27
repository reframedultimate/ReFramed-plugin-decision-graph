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
};
