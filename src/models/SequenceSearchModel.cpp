#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"
#include "decision-graph/models/Query.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/hash40.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MetaData.hpp"
#include "rfcommon/Session.hpp"

// ----------------------------------------------------------------------------
SequenceSearchModel::SequenceSearchModel(const LabelMapper* labelMapper)
    : labelMapper_(labelMapper)
{}

// ----------------------------------------------------------------------------
int SequenceSearchModel::sessionCount() const
{
    return sessions_.count();
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::startNewSession(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata)
{
    // It's possible that the players switch fighters between games, 
    // especially when multiple sessions from different days are
    // accumulated. For this we set up a mapping table that maps the
    // session's fighter index to our own internal fighter index.
    //
    // We use the player's name AND fighter ID to make this mapping,
    // because it doesn't make sense to clump together frame data from
    // two different players when they play the same character, and it
    // doesn't make sense to clump together frame data from the same player
    // when he plays different characters.
    auto indexUsed = rfcommon::SmallVector<int, 8>::makeReserved(fighters_.count());
    auto unmapped = rfcommon::SmallVector<int, 8>::makeReserved(fighters_.count());
    fighterIdxMapFromSession_.resize(mdata->fighterCount());
    for (int s = 0; s != mdata->fighterCount(); ++s)
    {
        for (int i = 0; i != fighters_.count(); ++i)
        {
            if (indexUsed.findFirst(i) != indexUsed.end())
                continue;

            if (fighters_[i].id == mdata->fighterID(s) && fighters_[i].playerName == mdata->name(s))
            {
                fighterIdxMapFromSession_[s] = i;
                goto matched;
            }
        }
        unmapped.push(s);
        matched: continue;
    }

    const int newSessionCount = sessions_.count() + 1;
    const int newFighterCount = fighters_.count() + 1;

    // Create session
    auto v = rfcommon::Vector<Sequence>::makeReserved(newFighterCount);
    auto sessionData = sessions_.emplace(
        mdata->timeStarted(),
        std::move(v)
    );

    /*
    // Create new entries for remaining unmapped fighters
    for (int s : unmapped)
    {
        fighterIdxMapFromSession_[s] = fighters_.count();
        auto fighterData = fighters_.emplace(
            mdata->fighterID(s),
            mdata->name(s),
            map->fighter.toName(mdata->fighterID(s)),
            rfcommon::Vector<Sequence>::makeReserved(newSessionCount)
        );

        // For newly inserted fighters, sequence range will start at 0
        for (int i = 0; i != newSessionCount; ++i)
            fighterData.sessions.emplace(0, 0);
    }

    // For new sessions, point the session ranges to the end of each
    // sequence
    for (int i = 0; i != newFighterCount; ++i)
    {
        const int endIdx = fighters_[i].states.count();
        sessionData.fighters.emplace(endIdx, endIdx);
    }

    // Update size of results structure
    for (int i = 0; i != queryCount(); ++i)
    {
        queryResults_[i].sessionGraph.emplace();
        queryResults_[i].sessionMatches.emplace();
    }

    // If there is a fighter and player name in the current session that
    // matches the previous fighter, we will want to set that as the current
    // fighter. This is a small QoL that helps with scanning through replays.
    currentFighterIdx_ = 0;  // Make sure to never go out of bounds when switching between e.g. 1v1 and 2v2
    for (int s = 0; s != mdata->fighterCount(); ++s)
        if (fighters_[currentFighterIdx_].id == mdata->fighterID(s) && 
            fighters_[currentFighterIdx_].playerName == mdata->name(s))
        {
            currentFighterIdx_ = fighterIdxMapFromSession_[s];
                break;
        }

    dispatcher.dispatch(&SequenceSearchListener::onNewSession);*/
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::addFrameNoNotify(int frameIdx, const rfcommon::FrameData* fdata)
{
    frameCount_++;

    for (int sessionFighterIdx = 0; sessionFighterIdx != fdata->fighterCount(); ++sessionFighterIdx)
    {
        const auto& fighterState = fdata->stateAt(sessionFighterIdx, frameIdx);
        const int fighterIdx = fighterIdxMapFromSession_[sessionFighterIdx];

        const bool inHitlag = [this, fdata, fighterIdx, frameIdx, &fighterState]() -> bool {
            if (fdata->fighterCount() != 2)
                return false;

            const auto& opponentState = fdata->stateAt(1 - fighterIdx, frameIdx);
            const auto& prevFighterState = frameIdx > 0 ? fdata->stateAt(fighterIdx, frameIdx - 1) : fighterState;
            if (opponentState.flags().attackConnected())
                return fighterState.hitstun() == prevFighterState.hitstun();

            return false;
        }();

        const bool inHitstun = [&fighterState]() -> bool {
            return fighterState.hitstun() > 0;
        }();

        const bool inShieldlag = [&fighterState]() -> bool {
            return fighterState.status().value() == 30;  // FIGHTER_STATUS_KIND_GUARD_DAMAGE
        }();

        const bool opponentInHitlag = [this, fdata, fighterIdx, frameIdx, &fighterState]() -> bool {
            if (fdata->fighterCount() != 2)
                return false;

            const auto& opponentState = fdata->stateAt(1 - fighterIdx, frameIdx);
            const auto& prevOpponentState = frameIdx > 0 ? fdata->stateAt(1 - fighterIdx, frameIdx - 1) : opponentState;
            if (fighterState.flags().attackConnected())
                return opponentState.hitstun() == prevOpponentState.hitstun();

            return false;
        }();

        const bool opponentInHitstun = [fdata, fighterIdx, frameIdx]() -> bool {
            if (fdata->fighterCount() != 2)
                return false;

            const auto& opponentState = fdata->stateAt(1 - fighterIdx, frameIdx);
            return opponentState.hitstun() > 0;
        }();

        const bool opponentInShieldlag = [fdata, fighterIdx, frameIdx]() -> bool {
            if (fdata->fighterCount() != 2)
                return false;

            const auto& opponentState = fdata->stateAt(1 - fighterIdx, frameIdx);
            return opponentState.status().value() == 30;  // FIGHTER_STATUS_KIND_GUARD_DAMAGE
        }();

        const State state(
            State::SideData(
                fighterState.frameIndex(), 
                fighterState.pos(), 
                fighterState.damage(), 
                fighterState.shield()),
            fighterState.motion(),
            fighterState.status(),
            fighterState.hitStatus(),
            inHitlag, inHitstun, inShieldlag,
            opponentInHitlag, opponentInHitstun, opponentInShieldlag
        );

        // Only process state if it is meaningfully different from the previously
        // processed state
        States& states = fighters_[fighterIdx].states;
        if (states.count() > 0 && state.compareWithoutSideData(states.back()))
            continue;

        // Add state to fighter's global sequence (spans multiple sessions)
        states.push(state);

        // Update sequence ranges for current session
        Sequence& fighterSessionSeq = fighters_[fighterIdx].sessions.back();
        Sequence& sessionFighterSeq = sessions_.back().fighters[fighterIdx];
        fighterSessionSeq = Sequence(fighterSessionSeq.first(), states.count());
        sessionFighterSeq = Sequence(sessionFighterSeq.first(), states.count());

        /*
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
        */
    }
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::addFrame(int frameIdx, const rfcommon::FrameData* fdata)
{
    addFrameNoNotify(frameIdx, fdata);
    dispatcher.dispatch(&SequenceSearchListener::onDataAdded);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::addAllFrames(const rfcommon::FrameData* fdata)
{
    for (int f = 0; f != fdata->frameCount(); ++f)
        addFrameNoNotify(f, fdata);
    dispatcher.dispatch(&SequenceSearchListener::onDataAdded);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::clearAll()
{
    sessions_.clearCompact();
    fighters_.clearCompact();
    fighterIdxMapFromSession_.clearCompact();
    currentFighterIdx_ = -1;
    frameCount_ = 0;

    dispatcher.dispatch(&SequenceSearchListener::onDataCleared);
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::fighterCount() const
{
    return fighters_.count();
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::currentFighter() const
{
    return currentFighterIdx_;
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::setCurrentFighter(int fighterIdx)
{
    currentFighterIdx_ = fighterIdx;
    dispatcher.dispatch(&SequenceSearchListener::onCurrentFighterChanged);
}

// ----------------------------------------------------------------------------
const char* SequenceSearchModel::playerName(int fighterIdx) const
{
    return fighters_[fighterIdx].playerName.cStr();
}

// ----------------------------------------------------------------------------
const char* SequenceSearchModel::fighterName(int fighterIdx) const
{
    return fighters_[fighterIdx].fighterName.cStr();
}

// ----------------------------------------------------------------------------
rfcommon::FighterID SequenceSearchModel::fighterID(int fighterIdx) const
{
    return fighters_[fighterIdx].id;
}

// ----------------------------------------------------------------------------
const States& SequenceSearchModel::fighterStates(int fighterIdx) const
{
    return fighters_[fighterIdx].states;
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::queryCount() const
{
    return queries_.count();
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::addQuery()
{
    queries_.emplace();
    queryStrings_.emplace();
    auto& results = queryResults_.emplace();

    results.sessionGraph.resize(sessionCount());
    results.sessionMatches.resize(sessionCount());
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::removeQuery(int queryIdx)
{
    queries_.erase(queryIdx);
    queryStrings_.erase(queryIdx);
    queryResults_.erase(queryIdx);
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::setQuery(int queryIdx, const char* queryStr)
{
    QueryASTNode* ast;
    Query* query;

    if (fighterCount() == 0)
        return false;

    queries_[queryIdx].reset();
    queryStrings_[queryIdx] = queryStr;
    queryResults_[queryIdx].graph.clear();
    queryResults_[queryIdx].matches.clearCompact();

    ast = Query::parse(queryStr);
    if (ast == nullptr)
    {
        queryError_ = "Parse error";
        return false;
    }
    ast->exportDOT("query-ast.dot");

    const auto fighterID = fighters_[currentFighterIdx_].id;
    query = Query::compileAST(ast, labelMapper_, fighterID);
    QueryASTNode::destroyRecurse(ast);
    if (query == nullptr)
    {
        queryError_ = "Compile error";
        return false;
    }
    query->exportDOT("query.dot", labelMapper_, fighterID);

    queries_[queryIdx].reset(query);

    return true;
}

// ----------------------------------------------------------------------------
const char* SequenceSearchModel::queryStr(int queryIdx) const
{
    return queryStrings_[queryIdx].cStr();
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::applyQueryNoNotify(int queryIdx)
{
    if (queries_[queryIdx].get() == nullptr)
        return false;

    auto& results = queryResults_[queryIdx];
    auto& query = *queries_[queryIdx];

    const auto& states = fighters_[currentFighterIdx_].states;

    results.matches = query.apply(states, Sequence(0, states.count()));
    results.graph = Graph::fromSequences(states, results.matches);
    results.graph.exportDOT("decision_graph_search.dot", fighters_[currentFighterIdx_].id, labelMapper_);

    for (int sessionIdx = 0; sessionIdx != sessionCount(); ++sessionIdx)
    {
        const auto& sessionSeq = sessions_[sessionIdx].fighters[currentFighterIdx_];
        results.sessionMatches[sessionIdx] = query.apply(states, sessionSeq);
        results.sessionGraph[sessionIdx] = Graph::fromSequences(states, results.sessionMatches[sessionIdx]);
    }

    return true;
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::applyQuery(int queryIdx)
{
    if (applyQueryNoNotify(queryIdx))
    {
        dispatcher.dispatch(&SequenceSearchListener::onQueryApplied);
        return true;
    }
    return false;
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::applyAllQueries()
{
    bool success = true;
    for (int i = 0; i != queryCount(); ++i)
        success &= applyQuery(i);

    if (success)
        dispatcher.dispatch(&SequenceSearchListener::onQueryApplied);
    return success;
}

// ----------------------------------------------------------------------------
const char* SequenceSearchModel::lastQueryError() const
{
    return queryError_.cStr();
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::totalFrameCount() const
{
    return frameCount_;
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::totalSequenceLength() const
{
    return fighters_.count() ? fighters_[currentFighterIdx_].states.count() : 0;
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::totalMatchedSequences() const
{
    int count = 0;
    for (int i = 0; i != queryCount(); ++i)
        count += queryResults_[i].matches.count();
    return count;
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::totalMatchedStates() const
{
    int count = 0;
    for (int i = 0; i != queryCount(); ++i)
        for (const auto& seq : queryResults_[i].matches)
            count += seq.last() - seq.first();
    return count;
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::totalMatchedUniqueStates() const
{
    int count = 0;
    for (int i = 0; i != queryCount(); ++i)
        count += queryResults_[i].graph.nodes.count();
    return count;
}

// ----------------------------------------------------------------------------
const Graph& SequenceSearchModel::graph(int queryIdx) const
{
    return queryResults_[queryIdx].graph;
}

// ----------------------------------------------------------------------------
const rfcommon::Vector<Sequence>& SequenceSearchModel::matches(int queryIdx) const
{
    return queryResults_[queryIdx].matches;
}

// ----------------------------------------------------------------------------
const Graph& SequenceSearchModel::sessionGraph(int queryIdx, int sessionIdx) const
{
    return queryResults_[queryIdx].sessionGraph[sessionIdx];
}

// ----------------------------------------------------------------------------
const rfcommon::Vector<Sequence>& SequenceSearchModel::sessionMatches(int queryIdx, int sessionIdx) const
{
    return queryResults_[queryIdx].sessionMatches[sessionIdx];
}
