#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"
#include "decision-graph/models/Query.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "rfcommon/Frame.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/hash40.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/Metadata.hpp"
#include "rfcommon/Session.hpp"

#include "decision-graph/models/LabelMapper.hpp"
#include <cstdio>

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
void SequenceSearchModel::startNewSession(const rfcommon::MappingInfo* map, const rfcommon::Metadata* mdata)
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

            if (fighters_[i].states.fighterID == mdata->playerFighterID(s) && fighters_[i].playerName == mdata->playerTag(s))
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
            mdata->playerTag(s),
            map->fighter.toName(mdata->playerFighterID(s)),
            States(mdata->playerFighterID(s))
         });

        // Insert empty session ranges for the new fighters
        for (int i = 0; i != sessions_.count(); ++i)
            sessions_[i].fighters.emplace(0, 0);
    }

    // Create session
    auto sessionData = sessions_.push({
        mdata->timeStarted(),
        rfcommon::Vector<Range>::makeReserved(fighters_.count())
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
        queryResults_[i].sessionMergedMatches.emplace();
        queryResults_[i].sessionMergedAndNormalizedMatches.emplace();
    }

    // If there is a fighter and player name in the current session that
    // matches the previous fighter, we will want to set that as the current
    // fighter. This is a small QoL that helps with scanning through replays.
    if (currentFighterIdx_ >= 0 && currentFighterIdx_ < fighters_.count())
    {
        for (int s = 0; s != mdata->fighterCount(); ++s)
            if (fighters_[currentFighterIdx_].states.fighterID == mdata->playerFighterID(s) &&
                fighters_[currentFighterIdx_].playerName == mdata->playerTag(s))
            {
                currentFighterIdx_ = fighterIdxMapFromSession_[s];
                break;
            }
    }
    if (currentFighterIdx_ < 0 || currentFighterIdx_ >= fighters_.count())
    {
        for (int s = 0; s != mdata->fighterCount(); ++s)
            if (previousFighterID_ == mdata->playerFighterID(s) &&
                previousPlayerName_ == mdata->playerTag(s))
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
        const auto& prevFighterState = frameIdx > 0 ?
                fdata->stateAt(sessionFighterIdx, frameIdx - 1) : fighterState;
        const auto& opponentState = fdata->fighterCount() == 2 ?
                fdata->stateAt(1 - sessionFighterIdx, frameIdx) : fighterState;
        const auto& prevOpponentState = frameIdx > 0 ?
                fdata->fighterCount() == 2 ?
                    fdata->stateAt(1 - sessionFighterIdx, frameIdx - 1) : opponentState :
                prevFighterState;

        const bool inHitlag =
                opponentState.flags().attackConnected() &&
                fighterState.hitstun() == prevFighterState.hitstun();
        const bool inHitstun = fighterState.hitstun() > 0;
        const bool inShieldlag = fighterState.status().value() == 30;  // FIGHTER_STATUS_KIND_GUARD_DAMAGE
        const bool isRising = fighterState.pos().y() > prevFighterState.pos().y();
        const bool isFalling = fighterState.pos().y() < prevFighterState.pos().y();

        const bool opponentInHitlag =
                fighterState.flags().attackConnected() &&
                opponentState.hitstun() == prevOpponentState.hitstun();
        const bool opponentInHitstun = opponentState.hitstun() > 0;
        const bool opponentInShieldlag = opponentState.status().value() == 30;  // FIGHTER_STATUS_KIND_GUARD_DAMAGE

        // TODO: Bury state for ZSS:
        //    FIGHTER_STATUS_KIND_TREAD_DAMAGE (185)   0x97eacd12e   (unknown)     kickflip_bury
        //    FIGHTER_STATUS_KIND_BURY (200)           0xb0a145fb5   damage_lw_3   Hitstun, hitstun
        //    FIGHTER_STATUS_KIND_BURY_WAIT (201)      0xb0a145fb5   damage_lw_3   Hitstun, hitstun
        //    FIGHTER_STATUS_KIND_BURY_JUMP (202)      0x62dd02058   jump_f        Jump, jump, fh

        // TODO: Command grabs
        //    FIGHTER_STATUS_KIND_CAPTURE_YOSHI (243)  0x112f7cb4d1  (unknown)     grabbed, grabbed_pull
        //    FIGHTER_STATUS_KIND_YOSHI_EGG (244)      0x93adf9e2e   (unknown)     (unlabeled)
        //    FIGHTER_STATUS_KIND_YOSHI_EGG (244)      0x7fb997a80   invalid       (unlabeled)

        // TODO: Phantom footstools:
        //    FIGHTER_STATUS_KIND_JUMP (11)            HIT_STATUS_XLU
        //    FIGHTER_STATUS_KIND_JUMP (11)            HIT_STATUS_NORMAL

        // Only add state if it is meaningfully different from the previously
        // added state
        const int fighterIdx = fighterIdxMapFromSession_[sessionFighterIdx];
        States& states = fighters_[fighterIdx].states;
        if (states.count() > 0 &&
                fighterState.motion() == states.back().motion &&
                fighterState.status() == states.back().status)
        {
            // It's possible that a move starts before it hits a shield/hits
            // an opponent. If this happens, we update the already added state
            // to include this flag
            states.back().flags |= State::makeFlags(inHitlag, false, inShieldlag, opponentInHitlag, false, opponentInShieldlag);
            continue;
        }

        // Add state to fighter's global sequence (spans multiple sessions)
        states.push(State(
            State::SideData(
                fighterState.frameIndex(),
                fighterState.pos(),
                fighterState.damage(),
                fighterState.shield()),
            fighterState.motion(),
            fighterState.status(),
            opponentState.motion(),
            opponentState.status(),
            inHitlag, inHitstun, inShieldlag,
            opponentInHitlag, opponentInHitstun, opponentInShieldlag
        ));

        // Update sequence ranges for current session
        Range& sessionFighterSeq = sessions_.back().fighters[fighterIdx];
        sessionFighterSeq.endIdx = states.count();
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
        queryResults_[i].mergedMatches.clearCompact();
        queryResults_[i].mergedAndNormalizedMatches.clearCompact();
        queryResults_[i].sessionGraph.clearCompact();
        queryResults_[i].sessionMatches.clearCompact();
        queryResults_[i].sessionMergedMatches.clearCompact();
        queryResults_[i].sessionMergedAndNormalizedMatches.clearCompact();
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
    results.sessionMergedMatches.resize(sessionCount());
    results.sessionMergedAndNormalizedMatches.resize(sessionCount());
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

    results.matches = query.apply(states, Range(0, states.count()));
    results.mergedMatches = query.mergeMotions(states, results.matches);
    results.mergedAndNormalizedMatches = query.normalizeMotions(states, results.mergedMatches);
    results.graph = Graph::fromSequences(states, results.mergedAndNormalizedMatches);
    Graph::fromSequences(states, results.mergedMatches).exportDOT("decision_graph_search.dot", fighters_[currentFighterIdx_].states, labelMapper_);

#ifndef NDEBUG
    printf("\nresults.matches:\n");
    for (const auto& range : results.matches)
    {
        printf("  ");
        for (int i = range.startIdx; i != range.endIdx; ++i)
        {
            if (i != range.startIdx)
                printf(" -> ");
            printf("0x%lx (%s)", states[i].motion.value(), labelMapper_->hash40StringOrHex(states[i].motion).cStr());
        }
        printf("\n");
    }
    printf("\nresults.mergedMatches:\n");
    for (const auto& seq : results.mergedMatches)
    {
        printf("  ");
        for (int i = 0; i != seq.idxs.count(); ++i)
        {
            int idx = seq.idxs[i];
            if (i != 0)
                printf(" -> ");
            printf("0x%lx (%s)", states[idx].motion.value(), labelMapper_->hash40StringOrHex(states[idx].motion).cStr());
        }
        printf("\n");
    }
    printf("\nresults.mergedAndNormalizedMatches:\n");
    for (const auto& seq : results.mergedAndNormalizedMatches)
    {
        printf("  ");
        for (int i = 0; i != seq.idxs.count(); ++i)
        {
            int idx = seq.idxs[i];
            if (i != 0)
                printf(" -> ");
            printf("0x%lx (%s)", states[idx].motion.value(), labelMapper_->hash40StringOrHex(states[idx].motion).cStr());
        }
        printf("\n");
    }
#endif

    for (int sessionIdx = 0; sessionIdx != sessionCount(); ++sessionIdx)
    {
        const auto& sessionSeq = sessions_[sessionIdx].fighters[currentFighterIdx_];
        results.sessionMatches[sessionIdx] = query.apply(states, sessionSeq);
        results.sessionMergedMatches[sessionIdx] = query.mergeMotions(states, results.sessionMatches[sessionIdx]);
        results.sessionMergedAndNormalizedMatches[sessionIdx] = query.normalizeMotions(states, results.sessionMergedMatches[sessionIdx]);
        results.sessionGraph[sessionIdx] = Graph::fromSequences(states, results.sessionMergedAndNormalizedMatches[sessionIdx]);
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
        for (const auto& range : queryResults_[i].matches)
            count += range.endIdx - range.startIdx - 1;
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
const rfcommon::Vector<Range>& SequenceSearchModel::matches(int queryIdx) const
{
    return queryResults_[queryIdx].matches;
}

// ----------------------------------------------------------------------------
const rfcommon::Vector<Sequence>& SequenceSearchModel::mergedMatches(int queryIdx) const
{
    return queryResults_[queryIdx].mergedMatches;
}

// ----------------------------------------------------------------------------
const Graph& SequenceSearchModel::sessionGraph(int queryIdx, int sessionIdx) const
{
    return queryResults_[queryIdx].sessionGraph[sessionIdx];
}

// ----------------------------------------------------------------------------
const rfcommon::Vector<Range>& SequenceSearchModel::sessionMatches(int queryIdx, int sessionIdx) const
{
    return queryResults_[queryIdx].sessionMatches[sessionIdx];
}
