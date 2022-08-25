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
    , previousFighterID_(rfcommon::FighterID::makeInvalid())
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
    auto unmapped = rfcommon::SmallVector<int, 8>::makeReserved(mdata->fighterCount());
    fighterIdxMapFromSession_.resize(mdata->fighterCount());
    for (int s = 0; s != mdata->fighterCount(); ++s)
    {
        for (int i = 0; i != fighters_.count(); ++i)
        {
            if (indexUsed.findFirst(i) != indexUsed.end())
                continue;

            if (fighters_[i].states.fighterID == mdata->fighterID(s) && fighters_[i].playerName == mdata->name(s))
            {
                fighterIdxMapFromSession_[s] = i;
                goto matched;
            }
        }
        unmapped.push(s);
        matched: continue;
    }

    // Create new entries for unmapped fighters
    for (int s : unmapped)
    {
        fighterIdxMapFromSession_[s] = fighters_.count();
        auto fighterData = fighters_.push({
            mdata->name(s),
            map->fighter.toName(mdata->fighterID(s)),
            States(mdata->fighterID(s))
         });

        // Insert empty session ranges for the new fighters
        for (int i = 0; i != sessions_.count(); ++i)
            sessions_[i].fighters.emplace(0, 0);
    }

    // Create session
    auto sessionData = sessions_.push({
        mdata->timeStarted(),
        rfcommon::Vector<Sequence>::makeReserved(fighters_.count())
    });

    // For new sessions, point the session ranges to the end of each
    // sequence
    for (int i = 0; i != fighters_.count(); ++i)
    {
        const int endIdx = fighters_[i].states.count();
        sessionData->fighters.emplace(endIdx, endIdx);
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
    if (currentFighterIdx_ >= 0 && currentFighterIdx_ < fighters_.count())
    {
        for (int s = 0; s != mdata->fighterCount(); ++s)
            if (fighters_[currentFighterIdx_].states.fighterID == mdata->fighterID(s) &&
                fighters_[currentFighterIdx_].playerName == mdata->name(s))
            {
                currentFighterIdx_ = fighterIdxMapFromSession_[s];
                break;
            }
    }
    if (currentFighterIdx_ < 0 || currentFighterIdx_ >= fighters_.count())
    {
        for (int s = 0; s != mdata->fighterCount(); ++s)
            if (previousFighterID_ == mdata->fighterID(s) &&
                previousPlayerName_ == mdata->name(s))
            {
                currentFighterIdx_ = fighterIdxMapFromSession_[s];
                break;
            }
    }

    // Make sure to never go out of bounds when switching between e.g. 1v1 and 2v2
    if (currentFighterIdx_ < 0 || currentFighterIdx_ >= fighters_.count())
        currentFighterIdx_ = 0;

    dispatcher.dispatch(&SequenceSearchListener::onNewSession);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::addFrameNoNotify(int frameIdx, const rfcommon::FrameData* fdata)
{
    frameCount_++;

    for (int sessionFighterIdx = 0; sessionFighterIdx != fdata->fighterCount(); ++sessionFighterIdx)
    {
        const auto& fighterState = fdata->stateAt(sessionFighterIdx, frameIdx);

        const bool inHitlag = [this, fdata, sessionFighterIdx, frameIdx, &fighterState]() -> bool {
            if (fdata->fighterCount() != 2)
                return false;

            const auto& opponentState = fdata->stateAt(1 - sessionFighterIdx, frameIdx);
            const auto& prevFighterState = frameIdx > 0 ? fdata->stateAt(sessionFighterIdx, frameIdx - 1) : fighterState;
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

        const bool opponentInHitlag = [this, fdata, sessionFighterIdx, frameIdx, &fighterState]() -> bool {
            if (fdata->fighterCount() != 2)
                return false;

            const auto& opponentState = fdata->stateAt(1 - sessionFighterIdx, frameIdx);
            const auto& prevOpponentState = frameIdx > 0 ? fdata->stateAt(1 - sessionFighterIdx, frameIdx - 1) : opponentState;
            if (fighterState.flags().attackConnected())
                return opponentState.hitstun() == prevOpponentState.hitstun();

            return false;
        }();

        const bool opponentInHitstun = [fdata, sessionFighterIdx, frameIdx]() -> bool {
            if (fdata->fighterCount() != 2)
                return false;

            const auto& opponentState = fdata->stateAt(1 - sessionFighterIdx, frameIdx);
            return opponentState.hitstun() > 0;
        }();

        const bool opponentInShieldlag = [fdata, sessionFighterIdx, frameIdx]() -> bool {
            if (fdata->fighterCount() != 2)
                return false;

            const auto& opponentState = fdata->stateAt(1 - sessionFighterIdx, frameIdx);
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
        const int fighterIdx = fighterIdxMapFromSession_[sessionFighterIdx];
        States& states = fighters_[fighterIdx].states;
        if (states.count() > 0 && state.compareWithoutSideData(states.back()))
            continue;

        // Add state to fighter's global sequence (spans multiple sessions)
        states.push(state);

        // Update sequence ranges for current session
        Sequence& sessionFighterSeq = sessions_.back().fighters[fighterIdx];
        sessionFighterSeq = Sequence(sessionFighterSeq.firstIdx(), states.count());
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

    for (int i = 0; i != queryCount(); ++i)
    {
        queryResults_[i].graph.clear();
        queryResults_[i].matches.clearCompact();
        queryResults_[i].sessionGraph.clearCompact();
        queryResults_[i].sessionMatches.clearCompact();
    }

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
    previousFighterID_ = fighters_[fighterIdx].states.fighterID;
    previousPlayerName_ = fighters_[fighterIdx].playerName;
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
    return fighters_[fighterIdx].states.fighterID;
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
void SequenceSearchModel::setQuery(int queryIdx, const char* queryStr)
{
    queries_[queryIdx].reset();
    queryStrings_[queryIdx] = queryStr;
    queryResults_[queryIdx].graph.clear();
    queryResults_[queryIdx].matches.clearCompact();
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::compileQuery(int queryIdx)
{
    if (fighterCount() == 0 || queryStrings_[queryIdx].length() == 0)
    {
        queryError_ = "No replay data loaded";
        return;
    }

    auto ast = Query::parse(queryStrings_[queryIdx].cStr());
    if (ast == nullptr)
    {
        queryError_ = "Parse error";
        return;
    }
    ast->exportDOT("query-ast.dot");

    const auto fighterID = fighters_[currentFighterIdx_].states.fighterID;
    auto query = Query::compileAST(ast, labelMapper_, fighterID);
    QueryASTNode::destroyRecurse(ast);
    if (query == nullptr)
    {
        queryError_ = "Compile error";
        return;
    }
    query->exportDOT("query.dot", labelMapper_, fighterID);

    queries_[queryIdx].reset(query);
    dispatcher.dispatch(&SequenceSearchListener::onQueryCompiled, queryIdx);
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::queryCompiled(int queryIdx) const
{
    return queries_[queryIdx].get() != nullptr;
}

// ----------------------------------------------------------------------------
const char* SequenceSearchModel::queryStr(int queryIdx) const
{
    return queryStrings_[queryIdx].cStr();
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::applyQueryNoNotify(int queryIdx)
{
    // May have to compile
    if (queries_[queryIdx].get() == nullptr)
        if (queryCompiled(queryIdx) == false)
            return false;

    auto& results = queryResults_[queryIdx];
    auto& query = *queries_[queryIdx];

    const auto& states = fighters_[currentFighterIdx_].states;

    results.matches = query.apply(states, Sequence(0, states.count()));
    results.graph = Graph::fromSequences(states, results.matches);
    results.graph.exportDOT("decision_graph_search.dot", fighters_[currentFighterIdx_].states, labelMapper_);

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
        success &= applyQueryNoNotify(i);

    if (success && queryCount() > 0)
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
            count += seq.count();
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
