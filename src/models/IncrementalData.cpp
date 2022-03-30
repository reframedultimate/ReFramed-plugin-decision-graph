#include "decision-graph/models/IncrementalData.hpp"
#include "decision-graph/listeners/IncrementalDataListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/hash40.hpp"

// ----------------------------------------------------------------------------
void IncrementalData::prepareNew(int fighterCount)
{
    sequences_.clearCompact();
    graphData_.clearCompact();

    sequences_.resize(fighterCount);
    graphData_.resize(fighterCount);

    frameCounter_ = 0;
}

// ----------------------------------------------------------------------------
void IncrementalData::addFrame(const rfcommon::Frame& frame)
{
    for (int fighterIdx = 0; fighterIdx != frame.fighterCount(); ++fighterIdx)
    {
        const auto& fighterState = frame.fighter(fighterIdx);
        GraphData& data = graphData_[fighterIdx];
        Sequence& seq = sequences_[fighterIdx];

        const State state(
            fighterState.motion(),
            fighterState.status(),
            fighterState.hitStatus(),
            true, true, true, true  // TODO
        );

        // Only process state if it is meaningfully different from the previously
        // processed state
        if (seq.states.count() > 0 && state == seq.states.back())
            continue;

        // Add to sequence
        seq.states.push(state);

        // New unique state
        auto nodeLookupResult = data.stateLookup.insertOrGet(state, -1);
        if (nodeLookupResult->value() == -1)
        {
            data.graph.nodes.emplace(state);
            nodeLookupResult->value() = data.graph.nodes.count() - 1;
        }
        const int currentNodeIdx = nodeLookupResult->value();

        // Create a new edge to the previously added/visited node
        if (data.prevNodeIdx != -1)
        {
            auto edgeLookupResult = data.edgeLookup.insertOrGet(EdgeConnection(data.prevNodeIdx, currentNodeIdx), -1);

            // If edge does not exist yet, create it. Otherwise, add weight to
            // the existing edge
            if (edgeLookupResult->value() == -1)
            {
                data.graph.edges.emplace(data.prevNodeIdx, currentNodeIdx);
                data.graph.nodes[data.prevNodeIdx].outgoingEdges.push(data.graph.edges.count() - 1);
                data.graph.nodes[currentNodeIdx].incomingEdges.push(data.graph.edges.count() - 1);

                edgeLookupResult->value() = data.graph.edges.count() - 1;
            }
            else
                data.graph.edges[edgeLookupResult->value()].addWeight();
        }
        data.prevNodeIdx = currentNodeIdx;
    }

    frameCounter_++;
}

// ----------------------------------------------------------------------------
void IncrementalData::notifyNewStatsAvailable()
{
    dispatcher.dispatch(&IncrementalDataListener::onIncrementalDataNewStats);
}

// ----------------------------------------------------------------------------
int IncrementalData::numFrames() const
{
    return frameCounter_;
}

// ----------------------------------------------------------------------------
void IncrementalData::buildExample1()
{
    State nair   = State(rfcommon::hash40("attack_air_n"), 0, 0, false, false, false, false);
    State utilt  = State(rfcommon::hash40("attack_hi_3"), 0, 0, false, false, false, false);
    State jump   = State(2, 0, 0, false, false, false, false);
    State land   = State(3, 0, 0, false, false, false, false);
    State roll   = State(4, 0, 0, false, false, false, false);
    State shield = State(5, 0, 0, false, false, false, false);
    State grab   = State(6, 0, 0, false, false, false, false);

    GraphData& data = graphData_[0];
    Sequence& seq = sequences_[0];
    seq.states.clear();
    data.graph.nodes.clear();
    data.graph.edges.clear();

    // Input sequence
    seq.states.push(jump);
    seq.states.push(nair);
    seq.states.push(land);
    seq.states.push(roll);
    seq.states.push(jump);
    seq.states.push(nair);
    seq.states.push(land);
    seq.states.push(shield);
    seq.states.push(jump);
    seq.states.push(nair);
    seq.states.push(land);
    seq.states.push(grab);
    seq.states.push(jump);
    seq.states.push(nair);
    seq.states.push(land);
    seq.states.push(utilt);
    seq.states.push(shield);

    // All nodes
    data.graph.nodes.emplace(nair);   // 0
    data.graph.nodes.emplace(utilt);  // 1
    data.graph.nodes.emplace(jump);   // 2
    data.graph.nodes.emplace(land);   // 3
    data.graph.nodes.emplace(roll);   // 4
    data.graph.nodes.emplace(shield); // 5
    data.graph.nodes.emplace(grab);   // 6

    // Unique edges
    data.graph.edges.emplace(2, 0);  // 0: jump -> nair
    data.graph.edges.emplace(0, 3);  // 1: nair -> land
    data.graph.edges.emplace(3, 4);  // 2: land -> roll
    data.graph.edges.emplace(4, 2);  // 3: roll -> jump
    data.graph.edges.emplace(3, 5);  // 4: land -> shield
    data.graph.edges.emplace(5, 2);  // 5: shield -> jump
    data.graph.edges.emplace(3, 6);  // 6: land -> grab
    data.graph.edges.emplace(6, 2);  // 7: grab -> jump
    data.graph.edges.emplace(3, 1);  // 8: land -> utilt
    data.graph.edges.emplace(1, 5);  // 9: utilt -> shield

    // Edge indices
    data.graph.nodes[0].outgoingEdges.push(1);  // nair -> land
    data.graph.nodes[1].outgoingEdges.push(9);  // utilt -> shield
    data.graph.nodes[2].outgoingEdges.push(0);  // jump -> nair
    data.graph.nodes[3].outgoingEdges.push(2);  // land -> roll
    data.graph.nodes[3].outgoingEdges.push(4);  // land -> shield
    data.graph.nodes[3].outgoingEdges.push(6);  // land -> grab
    data.graph.nodes[3].outgoingEdges.push(8);  // land -> utilt
    data.graph.nodes[4].outgoingEdges.push(3);  // roll -> jump
    data.graph.nodes[5].outgoingEdges.push(5);  // shield -> jump
    data.graph.nodes[6].outgoingEdges.push(7);  // grab -> jump
}

