#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "rfcommon/String.hpp"
#include <QGraphicsScene>

class SequenceSearchModel;
class Graph;

namespace rfcommon {
    class MotionLabels;
}

class GraphModel
        : public QGraphicsScene
        , public SequenceSearchListener
{
public:
    enum MergeBehavior
    {
        DONT_MERGE,
        QUERY_MERGE,
        LABEL_MERGE
    };

    GraphModel(SequenceSearchModel* searchModel, rfcommon::MotionLabels* labels);
    ~GraphModel();

    void setMergeBehavior(MergeBehavior behavior);
    void setPreferredLayer(int layerIdx);
    void setOutgoingTreeSize(int size);
    void setIncomingTreeSize(int size);
    void setUseLargestIsland(bool enable);
    void setFixEdges(bool enable);
    void setMergeQualifiers(bool enable);
    void setShowHash40Values(bool enable);
    void setShowQualifiers(bool enable);

    MergeBehavior mergeBehavior() const { return mergeBehavior_; }
    int preferredLayer() const { return preferredLayer_; }
    int outgoingTreeSize() const { return outgoingTree_; }
    int incomingTreeSize() const { return incomingTree_; }
    bool useLargestIsland() const { return useLargestIsland_; }
    bool showHash40Values() const { return showHash40Values_; }
    bool showQualifiers() const { return showQualifiers_; }

    int availableLayersCount() const;
    rfcommon::String availableLayerName(int idx) const;

private:
    void redrawGraph();

private:
    void onNewSessions() override;
    void onClearAll() override;
    void onDataAdded() override;
    void onPOVChanged() override;
    void onQueriesChanged() override;
    void onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) override;
    void onQueriesApplied() override;

private:
    SequenceSearchModel* searchModel_;
    rfcommon::MotionLabels* labels_;

    MergeBehavior mergeBehavior_ = QUERY_MERGE;
    int preferredLayer_ = -1;
    int outgoingTree_ = 0;
    int incomingTree_ = 0;
    bool useLargestIsland_ = false;
    bool fixEdges_ = true;
    bool mergeQualifiers_ = false;
    bool showHash40Values_ = false;
    bool showQualifiers_ = true;
};
