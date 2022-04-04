#include "decision-graph/models/IncrementalData.hpp"
#include "decision-graph/listeners/IncrementalDataListener.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/hash40.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
void IncrementalData::setSession(rfcommon::Session* session)
{
    sequences_.clearCompact();
    graphData_.clearCompact();

    sequences_.resize(session->fighterCount());
    graphData_.resize(session->fighterCount());

    frameCounter_ = 0;
    session_ = session;
}

// ----------------------------------------------------------------------------
void IncrementalData::clearSession(rfcommon::Session* session)
{
    session_ = nullptr;
}

// ----------------------------------------------------------------------------
void IncrementalData::addFrame(int frameIdx, const rfcommon::Frame& frame)
{
    for (int fighterIdx = 0; fighterIdx != frame.fighterCount(); ++fighterIdx)
    {
        const auto& fighterState = frame.fighter(fighterIdx);
        GraphData& data = graphData_[fighterIdx];
        Sequence& seq = sequences_[fighterIdx];

        const bool inHitlag = [this, fighterIdx, frameIdx, &fighterState]() -> bool {
            if (session_->fighterCount() != 2)
                return false;

            const auto& opponentState = session_->state(frameIdx, 1 - fighterIdx);
            const auto& prevFighterState = frameIdx > 0 ? session_->state(frameIdx - 1, fighterIdx) : fighterState;
            if (opponentState.flags().attackConnected())
                return fighterState.hitstun() == prevFighterState.hitstun();

            return false;
        }();

        const bool inHitstun = [&fighterState]() -> bool {
            return fighterState.hitstun() > 0;
        }();

        const bool inShieldlag = [&fighterState]() -> bool {
            return fighterState.status() == 30;  // FIGHTER_STATUS_KIND_GUARD_DAMAGE
        }();

        const bool opponentInHitlag = [this, fighterIdx, frameIdx, &fighterState]() -> bool {
            if (session_->fighterCount() != 2)
                return false;

            const auto& opponentState = session_->state(frameIdx, 1 - fighterIdx);
            const auto& prevOpponentState = frameIdx > 0 ? session_->state(frameIdx - 1, 1 - fighterIdx) : opponentState;
            if (fighterState.flags().attackConnected())
                return opponentState.hitstun() == prevOpponentState.hitstun();

            return false;
        }();

        const bool opponentInHitstun = [fighterIdx, &frame]() -> bool {
            if (frame.fighterCount() != 2)
                return false;

            const auto& opponentState = frame.fighter(1 - fighterIdx);
            return opponentState.hitstun() > 0;
        }();

        const bool opponentInShieldlag = [fighterIdx, &frame]() -> bool {
            if (frame.fighterCount() != 2)
                return false;

            const auto& opponentState = frame.fighter(1 - fighterIdx);
            return opponentState.status() == 30;  // FIGHTER_STATUS_KIND_GUARD_DAMAGE
        }();

        const State state(
            fighterState.motion(),
            fighterState.status(),
            fighterState.hitStatus(),
            inHitlag, inHitstun, inShieldlag,
            opponentInHitlag, opponentInHitstun, opponentInShieldlag
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
