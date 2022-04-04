#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/Graph.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

namespace rfcommon {
    class Frame;
}

class IncrementalDataListener;

class IncrementalData
{
public:
    void setSession(rfcommon::Session* session);
    void clearSession(rfcommon::Session* session);

    rfcommon::Session* session() const
        { return session_; }

    void prepareNew(int fighterCount);
    void addFrame(int frameIdx, const rfcommon::Frame& frame);
    void notifyNewStatsAvailable();

    int numFrames() const;

    const Sequence& sequence(int fighterIdx) const
        { return sequences_[fighterIdx]; }

    const Graph& graph(int fighterIdx) const
        { return graphData_[fighterIdx].graph; }

    rfcommon::ListenerDispatcher<IncrementalDataListener> dispatcher;

private:
    struct GraphData
    {
        Graph graph;
        rfcommon::HashMap<State, int, State::Hasher> stateLookup;
        rfcommon::HashMap<EdgeConnection, int, EdgeConnection::Hasher> edgeLookup;
        int prevNodeIdx = -1;
    };

    rfcommon::Reference<rfcommon::Session> session_;
    rfcommon::SmallVector<Sequence, 2> sequences_;
    rfcommon::SmallVector<GraphData, 2> graphData_;
    int frameCounter_ = 0;
};
