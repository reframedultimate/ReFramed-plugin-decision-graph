#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"
#include "decision-graph/models/Query.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

#include "rfcommon/FighterState.hpp"
#include "rfcommon/FrameData.hpp"
#include "rfcommon/GameMetadata.hpp"
#include "rfcommon/MappingInfo.hpp"
#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/ReplayFilename.hpp"

#include <cstdio>

// ----------------------------------------------------------------------------
SequenceSearchModel::SequenceSearchModel(const rfcommon::MotionLabels* labels)
    : labels_(labels)
    , previousFighterID_(rfcommon::FighterID::makeInvalid())
    , previousOpponentID_(rfcommon::FighterID::makeInvalid())
{}

// ----------------------------------------------------------------------------
void SequenceSearchModel::startNewSession(const rfcommon::MappingInfo* map, const rfcommon::Metadata* mdata)
{
    // Returns the player's name if possible, otherwise fall back to
    // player tag
    auto getPlayerName = [](const rfcommon::Metadata* mdata, int fighterIdx) -> const rfcommon::String& {
        switch (mdata->type())
        {
            case rfcommon::Metadata::GAME:
                if (const auto& name = mdata->asGame()->playerName(fighterIdx); name.length() > 0)
                    return name;
                break;
            case rfcommon::Metadata::TRAINING:
                break;
        }

        return mdata->playerTag(fighterIdx);
    };

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
    fighterIdxMapFromSession_.resize(mdata->fighterCount());
    for (int i = 0; i != mdata->fighterCount(); ++i)
    {
        for (int j = 0; j != fighterStates_.count(); ++j)
            if (fighterStates_[j].fighterID == mdata->playerFighterID(i) &&
                fighterStates_[j].playerName == getPlayerName(mdata, i))
            {
                fighterIdxMapFromSession_[i] = j;
                goto skip_add;
            }

        // Create new fighter entry with ID and name
        fighterIdxMapFromSession_[i] = fighterStates_.count();
        fighterStates_.emplace(
            mdata->playerFighterID(i),
            getPlayerName(mdata, i),
            map->fighter.toName(mdata->playerFighterID(i))
        );

        // We need to make sure that for the existing sessions, we insert
        // empty ranges so the vectors always have the same size as each other.
        for (int i = 0; i != sessions_.count(); ++i)
            sessions_[i].fighterStatesRange.emplace(0, 0);

    skip_add:;
    }

    // Create session
    auto sessionData = sessions_.push({
        mdata->timeStarted(),
        rfcommon::Vector<Range>::makeReserved(fighterStates_.count()),
        rfcommon::ReplayFilename::fromMetadata(map, mdata)
    });

    // For new sessions, point the session ranges to the end of each
    // sequence
    for (int i = 0; i != fighterStates_.count(); ++i)
    {
        const int endIdx = fighterStates_[i].count();
        sessionData->fighterStatesRange.emplace(endIdx, endIdx);
    }

    // Update size of results structure
    for (int i = 0; i != queryCount(); ++i)
    {
        queryResults_[i].sessionMatches.emplace();
        queryResults_[i].sessionMergedMatches.emplace();
        queryResults_[i].sessionMergedAndNormalizedMatches.emplace();
    }

    // If there is a fighter and player name in the current session that
    // matches the previous fighter, we will want to set that as the current
    // fighter. This is a small QoL that helps with scanning through replays.
    if (playerPOV_ >= 0 && playerPOV_ < fighterStates_.count())
    {
        for (int i = 0; i != mdata->fighterCount(); ++i)
            if (fighterStates_[playerPOV_].fighterID == mdata->playerFighterID(i) &&
                fighterStates_[playerPOV_].playerName == getPlayerName(mdata, i))
            {
                playerPOV_ = fighterIdxMapFromSession_[i];
                break;
            }
    }
    if (playerPOV_ < 0 || playerPOV_ >= fighterStates_.count())
    {
        for (int i = 0; i != mdata->fighterCount(); ++i)
            if (previousFighterID_ == mdata->playerFighterID(i) &&
                previousPlayerName_ == getPlayerName(mdata, i))
            {
                playerPOV_ = fighterIdxMapFromSession_[i];
                break;
            }
    }

    // Make sure to never go out of bounds when switching between e.g. 1v1 and 2v2
    if (playerPOV_ < 0 || playerPOV_ >= fighterStates_.count())
        playerPOV_ = 0;

    // TODO
    opponentPOV_ = 1 - playerPOV_;
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::notifyNewSessions()
{
    dispatcher.dispatch(&SequenceSearchListener::onNewSessions);
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::sessionCount() const
{
    return sessions_.count();
}

// ----------------------------------------------------------------------------
const char* SequenceSearchModel::sessionName(int sessionIdx) const
{
    return sessions_[sessionIdx].sessionName.cStr();
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::clearAllAndNotify()
{
    sessions_.clearCompact();
    fighterStates_.clearCompact();
    fighterIdxMapFromSession_.clearCompact();
    playerPOV_ = -1;
    opponentPOV_ = -1;

    for (int i = 0; i != queryCount(); ++i)
    {
        queryResults_[i].matches.clearCompact();
        queryResults_[i].mergedMatches.clearCompact();
        queryResults_[i].mergedAndNormalizedMatches.clearCompact();
        queryResults_[i].sessionMatches.clearCompact();
        queryResults_[i].sessionMergedMatches.clearCompact();
        queryResults_[i].sessionMergedAndNormalizedMatches.clearCompact();
    }

    dispatcher.dispatch(&SequenceSearchListener::onClearAll);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::addFrame(int frameIdx, const rfcommon::FrameData* fdata)
{
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
        States& states = fighterStates_[fighterIdx];
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
        Range& sessionFighterSeq = sessions_.back().fighterStatesRange[fighterIdx];
        sessionFighterSeq.endIdx = states.count();
    }
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::addAllFrames(const rfcommon::FrameData* fdata)
{
    for (int f = 0; f != fdata->frameCount(); ++f)
        addFrame(f, fdata);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::notifyFramesAdded()
{
    dispatcher.dispatch(&SequenceSearchListener::onDataAdded);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::setPlayerPOV(int fighterIdx)
{
    playerPOV_ = fighterIdx;
    previousFighterID_ = fighterStates_[fighterIdx].fighterID;
    previousPlayerName_ = fighterStates_[fighterIdx].playerName;
    dispatcher.dispatch(&SequenceSearchListener::onPOVChanged);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::setOpponentPOV(int fighterIdx)
{
    opponentPOV_ = fighterIdx;
    previousFighterID_ = fighterStates_[fighterIdx].fighterID;
    previousPlayerName_ = fighterStates_[fighterIdx].playerName;
    dispatcher.dispatch(&SequenceSearchListener::onPOVChanged);
}

// ----------------------------------------------------------------------------
int SequenceSearchModel::addQuery()
{
    compiledQueries_.emplace();
    queryStrings_.emplace();
    auto& results = queryResults_.emplace();

    results.sessionMatches.resize(sessionCount());
    results.sessionMergedMatches.resize(sessionCount());
    results.sessionMergedAndNormalizedMatches.resize(sessionCount());

    return compiledQueries_.count() - 1;
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::setQuery(int queryIdx, const char* queryStr, const char* oppQueryStr)
{
    compiledQueries_[queryIdx].player.reset();
    compiledQueries_[queryIdx].opponent.reset();
    queryStrings_[queryIdx] = { queryStr, oppQueryStr };
    queryResults_[queryIdx].matches.clearCompact();
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::removeQuery(int queryIdx)
{
    compiledQueries_.erase(queryIdx);
    queryStrings_.erase(queryIdx);
    queryResults_.erase(queryIdx);
}
// ----------------------------------------------------------------------------
void SequenceSearchModel::notifyQueriesChanged()
{
    dispatcher.dispatch(&SequenceSearchListener::onQueriesChanged);
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::compileQuery(int queryIdx)
{
    if (playerPOV_ < 0 || opponentPOV_ < 0 || queryStrings_[queryIdx].player.length() == 0)
    {
        dispatcher.dispatch(&SequenceSearchListener::onQueryCompiled,
            queryIdx,
            false, playerPOV_ < 0 || opponentPOV_ < 0 ? "No frame data loaded" : "Query string empty",
            false, playerPOV_ < 0 || opponentPOV_ < 0 ? "No frame data loaded" : "Query string empty");
        return false;
    }

    const rfcommon::String& playerStr = queryStrings_[queryIdx].player;
    const rfcommon::String& oppStr = queryStrings_[queryIdx].opponent;

    // Parse strings into ASTs
    QueryASTNode* ast = Query::parse(queryStrings_[queryIdx].player);
    QueryASTNode* oppAst = oppStr.length() ? Query::parse(oppStr) : nullptr;

    if (ast == nullptr || (oppStr.length() && oppAst == nullptr))
    {
        bool sucess = ast != nullptr;
        bool oppSuccess = !(oppStr.length() && oppAst == nullptr);
        dispatcher.dispatch(&SequenceSearchListener::onQueryCompiled,
            queryIdx,
            sucess, sucess ? "" : "Syntax Error",
            oppSuccess, oppSuccess ? "" : "Syntax Error");
        return false;
    }
    ast->exportDOT("query-ast.dot");
    if (oppAst)
        oppAst->exportDOT("query-opp-ast.dot");

    // Compile ASTs into NFAs
    rfcommon::String queryError, oppQueryError;
    std::unique_ptr<Query> query(Query::compileAST(ast, labels_, fighterID(playerPOV_), &queryError));
    std::unique_ptr<Query> oppQuery(oppAst ? Query::compileAST(oppAst, labels_, fighterID(opponentPOV_), &oppQueryError) : nullptr);
    QueryASTNode::destroyRecurse(ast);
    if (oppAst)
        QueryASTNode::destroyRecurse(oppAst);

    if (query == nullptr || (oppStr.length() && oppQuery == nullptr))
    {
        bool sucess = query != nullptr;
        bool oppSuccess = !(oppStr.length() && oppQuery == nullptr);
        dispatcher.dispatch(&SequenceSearchListener::onQueryCompiled,
            queryIdx,
            sucess, sucess ? "" : queryError.cStr(),
            oppSuccess, oppSuccess ? "" : oppQueryError.cStr());
        return false;
    }
    query->exportDOT("query.dot", labels_, fighterID(playerPOV_));
    if (oppQuery)
        oppQuery->exportDOT("query-opp.dot", labels_, fighterID(opponentPOV_));

    compiledQueries_[queryIdx].player = std::move(query);
    compiledQueries_[queryIdx].opponent = std::move(oppQuery);

    dispatcher.dispatch(&SequenceSearchListener::onQueryCompiled, queryIdx, true, "", true, "");
    return true;
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::compileAllQueries()
{
    bool success = queryCount() > 0;
    for (int i = 0; i != queryCount(); ++i)
        success &= compileQuery(i);
    return success;
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::applyQuery(int queryIdx)
{
    auto& results = queryResults_[queryIdx];
    auto& query = compiledQueries_[queryIdx].player;
    auto& oppQuery = compiledQueries_[queryIdx].opponent;

    // Check if query was compiled
    if (compiledQueries_[queryIdx].player == nullptr)
        return false;

    // Check if we have POVs set
    if (playerPOV_ < 0 || opponentPOV_ < 0)
        return false;

    results.matches = query->apply(
        fighterStates_[playerPOV_],
        Range(0, fighterStates_[playerPOV_].count())
    );

    // If there is a query for the opponent, we want to apply the query to the
    // range of opponent states that occurred at the same time as our search
    // result. This is not as simple, because the location of the relevent
    // states in the opponent's "fighterStates_[x]" array will be different
    // than the location of the player's states.
    //
    // We first have to find the range of opponent states that come from the
    // same session as the states we found for the player, and then refine
    // the search until we find the start and end indices of states that have
    // the same frame index as the player states.
    if (oppQuery.get() != nullptr)
    {
        results.matches.retain([this, &oppQuery](const Range& range) -> bool {
            // Find the session range of the opponent states to use as the initial
            // guess
            int sessionIdx;
            for (sessionIdx = 0; sessionIdx != sessionCount(); ++sessionIdx)
                if (range.startIdx >= sessions_[sessionIdx].fighterStatesRange[playerPOV_].startIdx &&
                    range.endIdx <= sessions_[sessionIdx].fighterStatesRange[playerPOV_].endIdx)
                {
                    break;
                }
            assert(sessionIdx != sessionCount());

            // We know which session the player's range came from -> use the
            // session range as the initial guess
            Range oppRange = sessions_[sessionIdx].fighterStatesRange[opponentPOV_];

            // Get the start and end frame index from the player states. Note that the range
            // end index could point outside of the playerStates array, and the playerStates
            // array could be empty.
            const States& player = fighterStates_[playerPOV_];
            rfcommon::FrameIndex startFrame = player.count() > 0 ?
                    player[range.startIdx].sideData.frameIndex :
                    rfcommon::FrameIndex::fromValue(0);
            rfcommon::FrameIndex endFrame = range.endIdx < player.count() ?
                    player[range.endIdx].sideData.frameIndex :
                    player.count() > 0 ?
                        player.back().sideData.frameIndex :
                        rfcommon::FrameIndex::fromValue(0);

            // Refine guess
            oppRange.startIdx = std::lower_bound(
                fighterStates_[opponentPOV_].begin() + oppRange.startIdx,
                fighterStates_[opponentPOV_].begin() + oppRange.endIdx,
                fighterStates_[playerPOV_][range.startIdx],
                [](const State& a, const State& b) {
                    return a.sideData.frameIndex < b.sideData.frameIndex;
                }
            ) - fighterStates_[opponentPOV_].begin();

            // We actually want one state before
            if (oppRange.startIdx > sessions_[sessionIdx].fighterStatesRange[opponentPOV_].startIdx)
                oppRange.startIdx--;

            oppRange.endIdx = std::upper_bound(
                fighterStates_[opponentPOV_].begin() + oppRange.startIdx,
                fighterStates_[opponentPOV_].begin() + oppRange.endIdx,
                fighterStates_[playerPOV_][range.endIdx],
                [](const State& a, const State& b) {
                    return a.sideData.frameIndex < b.sideData.frameIndex;
                }
            ) - fighterStates_[opponentPOV_].begin();

            return oppQuery->apply(fighterStates_[opponentPOV_], oppRange).count() > 0;
        });
    }

    results.mergedMatches = query->mergeMotions(fighterStates_[playerPOV_], results.matches);
    results.mergedAndNormalizedMatches = query->normalizeMotions(fighterStates_[playerPOV_], results.mergedMatches);
    Graph::fromSequences(fighterStates_[playerPOV_], results.mergedMatches).exportDOT("decision_graph_search.dot", fighterStates_[playerPOV_], labels_);

#ifndef NDEBUG
    auto toHash40OrHex = [this](rfcommon::FighterMotion motion) -> rfcommon::String {
        if (const char* h40 = labels_->lookupHash40(motion))
            return h40;
        return motion.toHex();
    };

    printf("\nresults.matches:\n");
    for (const auto& range : results.matches)
    {
        printf("  ");
        for (int i = range.startIdx; i != range.endIdx; ++i)
        {
            if (i != range.startIdx)
                printf(" -> ");
            printf("0x%lx (%s)", fighterStates_[playerPOV_][i].motion.value(), toHash40OrHex(fighterStates_[playerPOV_][i].motion).cStr());
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
            printf("0x%lx (%s)", fighterStates_[playerPOV_][idx].motion.value(), toHash40OrHex(fighterStates_[playerPOV_][idx].motion).cStr());
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
            printf("0x%lx (%s)", fighterStates_[playerPOV_][idx].motion.value(), toHash40OrHex(fighterStates_[playerPOV_][idx].motion).cStr());
        }
        printf("\n");
    }
#endif

    for (int sessionIdx = 0; sessionIdx != sessionCount(); ++sessionIdx)
    {
        results.sessionMatches[sessionIdx].clear();
        const auto& sessionRange = sessions_[sessionIdx].fighterStatesRange[playerPOV_];
        for (const Range& range : results.matches)
            if (range.startIdx >= sessionRange.startIdx && range.endIdx <= sessionRange.endIdx)
                results.sessionMatches[sessionIdx].push(range);
        results.sessionMergedMatches[sessionIdx] = query->mergeMotions(fighterStates_[playerPOV_], results.sessionMatches[sessionIdx]);
        results.sessionMergedAndNormalizedMatches[sessionIdx] = query->normalizeMotions(fighterStates_[playerPOV_], results.sessionMergedMatches[sessionIdx]);
    }

    return true;
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::applyAllQueries()
{
    bool success = false;
    for (int i = 0; i != queryCount(); ++i)
        success |= applyQuery(i);

    return success;
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::notifyQueriesApplied()
{
    dispatcher.dispatch(&SequenceSearchListener::onQueriesApplied);
}
