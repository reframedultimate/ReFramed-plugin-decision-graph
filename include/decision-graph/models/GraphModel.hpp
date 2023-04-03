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
    enum GraphType
    {
        FULL_GRAPH,
        OUTGOING_TREE,
        INCOMING_TREE
    };

    enum MergeBehavior
    {
        DONT_MERGE,
        QUERY_MERGE,
        LABEL_MERGE
    };

    GraphModel(SequenceSearchModel* searchModel, rfcommon::MotionLabels* labels);
    ~GraphModel();

    void setGraphType(GraphType type);
    void setMergeBehavior(MergeBehavior behavior);
    void setPreferredLayer(int layerIdx);
    void setUseLargestIsland(bool enable);
    void setShowHash40Values(bool enable);
    void setShowQualifiers(bool enable);

    GraphType graphType() const { return graphType_; }
    MergeBehavior mergeBehavior() const { return mergeBehavior_; }
    int preferredLayer() const { return preferredLayer_; }
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

    GraphType graphType_;
    MergeBehavior mergeBehavior_;
    int preferredLayer_ = -1;
    bool useLargestIsland_;
    bool showHash40Values_;
    bool showQualifiers_;
};
