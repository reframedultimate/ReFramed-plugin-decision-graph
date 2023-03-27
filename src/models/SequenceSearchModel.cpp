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
    auto indexUsed = rfcommon::SmallVector<int, 8>::makeReserved(fighterStates_.count());
    auto unmapped = rfcommon::SmallVector<int, 8>::makeReserved(mdata->fighterCount());
    fighterIdxMapFromSession_.resize(mdata->fighterCount());
    for (int s = 0; s != mdata->fighterCount(); ++s)
    {
        for (int i = 0; i != fighterStates_.count(); ++i)
        {
            if (indexUsed.findFirst(i) != indexUsed.end())
                continue;

            if (fighterStates_[i].fighterID == mdata->playerFighterID(s) && fighterStates_[i].playerName == mdata->playerTag(s))
            {
                fighterIdxMapFromSession_[s] = i;
                goto matched;
            }
        }
        unmapped.push(s);
        matched: continue;
    }

    // Returns the player's name if possible, otherwise fall back to
    // player tag
    auto getPlayerName = [mdata](int fighterIdx) -> const rfcommon::String& {
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

    // Create new entries for unmapped fighters
    for (int s : unmapped)
    {
        fighterIdxMapFromSession_[s] = fighterStates_.count();
        fighterStates_.emplace(
            mdata->playerFighterID(s),
            getPlayerName(s),
            map->fighter.toName(mdata->playerFighterID(s))
        );

        // Insert empty session ranges for the new fighters
        for (int i = 0; i != sessions_.count(); ++i)
            sessions_[i].fighterStatesRange.emplace(0, 0);
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
    if (currentFighterIdx_ >= 0 && currentFighterIdx_ < fighterStates_.count())
    {
        for (int s = 0; s != mdata->fighterCount(); ++s)
            if (fighterStates_[currentFighterIdx_].fighterID == mdata->playerFighterID(s) &&
                fighterStates_[currentFighterIdx_].playerName == mdata->playerTag(s))
            {
                currentFighterIdx_ = fighterIdxMapFromSession_[s];
                break;
            }
    }
    if (currentFighterIdx_ < 0 || currentFighterIdx_ >= fighterStates_.count())
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
    if (currentFighterIdx_ < 0 || currentFighterIdx_ >= fighterStates_.count())
        currentFighterIdx_ = 0;

    // TODO
    opponentFighterIdx_ = 1 - currentFighterIdx_;
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
    currentFighterIdx_ = -1;
    opponentFighterIdx_ = -1;

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
    currentFighterIdx_ = fighterIdx;
    previousFighterID_ = fighterStates_[fighterIdx].fighterID;
    previousPlayerName_ = fighterStates_[fighterIdx].playerName;
    dispatcher.dispatch(&SequenceSearchListener::onPOVChanged);
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::setOpponentPOV(int fighterIdx)
{
    opponentFighterIdx_ = fighterIdx;
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
bool SequenceSearchModel::compileQuery(int queryIdx, rfcommon::FighterID fighterID, rfcommon::FighterID oppFighterID)
{
    if (fighterCount() == 0 || queryStrings_[queryIdx].player.length() == 0)
    {
        dispatcher.dispatch(&SequenceSearchListener::onQueryCompiled,
            queryIdx,
            false, fighterCount() == 0 ? "No frame data loaded" : "Query string empty",
            false, fighterCount() == 0 ? "No frame data loaded" : "Query string empty");
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
    std::unique_ptr<Query> query(Query::compileAST(ast, labels_, fighterID));
    std::unique_ptr<Query> oppQuery(oppAst ? Query::compileAST(oppAst, labels_, oppFighterID) : nullptr);
    QueryASTNode::destroyRecurse(ast);
    if (oppAst)
        QueryASTNode::destroyRecurse(oppAst);

    if (query == nullptr || (oppStr.length() && oppQuery == nullptr))
    {
        bool sucess = query != nullptr;
        bool oppSuccess = !(oppStr.length() && oppQuery == nullptr);
        dispatcher.dispatch(&SequenceSearchListener::onQueryCompiled,
            queryIdx,
            sucess, sucess ? "" : "Compile Error",
            oppSuccess, oppSuccess ? "" : "Compile Error");
        return false;
    }
    query->exportDOT("query.dot", labels_, fighterID);
    if (oppQuery)
        oppQuery->exportDOT("query-opp.dot", labels_, oppFighterID);

    compiledQueries_[queryIdx].player = std::move(query);
    compiledQueries_[queryIdx].opponent = std::move(oppQuery);

    dispatcher.dispatch(&SequenceSearchListener::onQueryCompiled, queryIdx, true, "", true, "");
    return true;
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::applyQuery(int queryIdx, const States& playerStates, const States& opponentStates)
{
    auto& results = queryResults_[queryIdx];
    auto& query = compiledQueries_[queryIdx].player;
    auto& oppQuery = compiledQueries_[queryIdx].opponent;

    // Check if query was compiled
    if (compiledQueries_[queryIdx].player == nullptr)
        return false;

    results.matches = query->apply(playerStates, Range(0, playerStates.count()));

    // If there is a query for the opponent, we want to apply the query to the
    // range of opponent states that occurred at the same time as our search
    // result
    if (oppQuery.get() != nullptr)
    {
        results.matches.retain([playerStates, opponentStates, &oppQuery](const Range& range) -> bool {
            // Get the start and end frame index from the player states. Note that the range
            // end index could point outside of the playerStates array, and the playerStates
            // array could be empty.
            rfcommon::FrameIndex startFrame = playerStates.count() > 0 ?
                    playerStates[range.startIdx].sideData.frameIndex :
                    rfcommon::FrameIndex::fromValue(0);
            rfcommon::FrameIndex endFrame = range.endIdx < playerStates.count() ?
                    playerStates[range.endIdx].sideData.frameIndex :
                    playerStates.count() > 0 ?
                        playerStates.back().sideData.frameIndex :
                        rfcommon::FrameIndex::fromValue(0);

            // Initial guess opponent index == player index
            Range oppRange(
                range.startIdx <= opponentStates.count() ?
                    range.startIdx :
                    opponentStates.count(),
                range.endIdx <= opponentStates.count() ?
                    range.endIdx :
                    opponentStates.count());

            // Refine guess
            while (oppRange.startIdx < opponentStates.count() - 1 &&
                opponentStates[oppRange.startIdx].sideData.frameIndex < startFrame)
            {
                oppRange.startIdx++;
            }
            while (oppRange.startIdx > 0 &&
                opponentStates[oppRange.startIdx].sideData.frameIndex > startFrame)
            {
                oppRange.startIdx--;
            }
            while (oppRange.endIdx < opponentStates.count() - 1 &&
                opponentStates[oppRange.endIdx].sideData.frameIndex < endFrame)
            {
                oppRange.endIdx++;
            }
            while (oppRange.endIdx > 0 &&
                opponentStates[oppRange.endIdx].sideData.frameIndex > endFrame)
            {
                oppRange.endIdx--;
            }

            return oppQuery->apply(opponentStates, oppRange).count() > 0;
        });
    }

    results.mergedMatches = query->mergeMotions(playerStates, results.matches);
    results.mergedAndNormalizedMatches = query->normalizeMotions(playerStates, results.mergedMatches);
    Graph::fromSequences(playerStates, results.mergedMatches).exportDOT("decision_graph_search.dot", playerStates, labels_);

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
            printf("0x%lx (%s)", playerStates[i].motion.value(), toHash40OrHex(playerStates[i].motion).cStr());
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
            printf("0x%lx (%s)", playerStates[idx].motion.value(), toHash40OrHex(playerStates[idx].motion).cStr());
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
            printf("0x%lx (%s)", playerStates[idx].motion.value(), toHash40OrHex(playerStates[idx].motion).cStr());
        }
        printf("\n");
    }
#endif

    for (int sessionIdx = 0; sessionIdx != sessionCount(); ++sessionIdx)
    {
        results.sessionMatches[sessionIdx].clear();
        const auto& sessionRange = sessions_[sessionIdx].fighterStatesRange[currentFighterIdx_];
        for (const Range& range : results.matches)
            if (range.startIdx >= sessionRange.startIdx && range.endIdx <= sessionRange.endIdx)
                results.sessionMatches[sessionIdx].push(range);
        results.sessionMergedMatches[sessionIdx] = query->mergeMotions(playerStates, results.sessionMatches[sessionIdx]);
        results.sessionMergedAndNormalizedMatches[sessionIdx] = query->normalizeMotions(playerStates, results.sessionMergedMatches[sessionIdx]);
    }

    return true;
}

// ----------------------------------------------------------------------------
bool SequenceSearchModel::applyAllQueries(const States& playerStates, const States& opponentStates)
{
    bool success = queryCount() > 0;
    for (int i = 0; i != queryCount(); ++i)
        success &= applyQuery(i, playerStates, opponentStates);

    return success;
}

// ----------------------------------------------------------------------------
void SequenceSearchModel::notifyQueriesApplied()
{
    dispatcher.dispatch(&SequenceSearchListener::onQueriesApplied);
}
