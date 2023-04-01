#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QGraphicsView>

class SequenceSearchModel;
class GraphModel;

namespace rfcommon {
    class MotionLabels;
}

class GraphView
        : public QGraphicsView
        , public SequenceSearchListener
{
    Q_OBJECT

public:
    explicit GraphView(GraphModel* graphModel, SequenceSearchModel* searchModel, rfcommon::MotionLabels* labels, QWidget* parent=nullptr);
    ~GraphView();

private:
    void onNewSessions() override;
    void onClearAll() override;
    void onDataAdded() override;
    void onPOVChanged() override;
    void onQueriesChanged() override;
    void onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) override;
    void onQueriesApplied() override;

private:
    GraphModel* graphModel_;
    SequenceSearchModel* searchModel_;
    rfcommon::MotionLabels* labels_;
};
