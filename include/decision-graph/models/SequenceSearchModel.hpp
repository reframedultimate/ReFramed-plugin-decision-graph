#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/Graph.hpp"

#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/TimeStamp.hpp"
#include "rfcommon/Vector.hpp"

#include <memory>

class Query;
class SequenceSearchListener;

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class Metadata;
    class MotionLabels;
}

class SequenceSearchModel
{
public:
    SequenceSearchModel(const rfcommon::MotionLabels* labels);

    /*
     * Allocates and prepares a new entry in the sessions structures. You
     * should call notifyNewSessions() after adding 1 or more sessions to
     * notify the UI. This will cause the UI to clear any data from previous
     * sessions and adopt new session data such as player names and fighters.
     */
    void startNewSession(const rfcommon::MappingInfo* map, const rfcommon::Metadata* mdata);
    void notifyNewSessions();
    int sessionCount() const;
    const char* sessionName(int sessionIdx) const;

    /*
     * Clears all data and notifies the UI to reset everything. This is mostly
     * used when replays are deselected, or if training mode is reset.
     */
    void clearAllAndNotify();

    /*
     * Adds a frame to the currently active session. You can call this multiple
     * times and then call notifyFramesAdded() to cause the UI to update.
     */
    void addFrame(int frameIdx, const rfcommon::FrameData* fdata);
    void addAllFrames(const rfcommon::FrameData* fdata);
    void notifyFramesAdded();

    /*
     * Controls for setting the player and opponent fighters. The UI is
     * automatically notified.
     *
     * Note that when changing fighters, queries need to be re-compiled,
     * because they depend on the fighterID.
     */
    void setPlayerPOV(int fighterIdx);
    void setOpponentPOV(int fighterIdx);
    int playerPOV() const { return playerPOV_; }
    int opponentPOV() const { return opponentPOV_; }
    int fighterCount() const { return fighterStates_.count(); }
    const rfcommon::String& playerName(int fighterIdx) const { return fighterStates_[fighterIdx].playerName; }
    const rfcommon::String& fighterName(int fighterIdx) const { return fighterStates_[fighterIdx].fighterName; }
    rfcommon::FighterID fighterID(int fighterIdx) const { return fighterStates_[fighterIdx].fighterID; }
    const States& fighterStates(int fighterIdx) const { return fighterStates_[fighterIdx]; }

    /*
     * Adding a query will create a new empty entry in the list of queries and
     * return the index. You should then set the search string via setQuery(),
     * and finally, call notifyQueriesChanged() to notify the UI. Changing the
     * search string does not compile or apply the query.
     *
     * It is possible to specify a query string for the opponent. This will
     * let you simultaneously search for player and opponent states. To
     * disable opponent search, set the opponent query string to an empty string
     * (not nullptr).
     */
    int addQuery();
    void setQuery(int queryIdx, const char* queryStr, const char* oppQueryStr);
    void removeQuery(int queryIdx);
    void notifyQueriesChanged();
    int queryCount() const { return compiledQueries_.count(); }
    const rfcommon::String& playerQuery(int queryIdx) const { return queryStrings_[queryIdx].player; }
    const rfcommon::String& opponentQuery(int queryIdx) const { return queryStrings_[queryIdx].opponent; }

    /*
     * Once the query strings are set, they need to be compiled. This needs to
     * be done whenever the search string changes, or if a fighter changes.
     *
     * This calls onQueryCompiled(). The success flag (and error message) are
     * passed to this callback. Additionally, if compilation succeeds, true is
     * returned.
     */
    bool compileQuery(int queryIdx);
    bool compileAllQueries();

    /*
     * Applies queries to the current data. Whenever frames are added, or new
     * sessions are added, or if a query is re-compiled, this should be called.
     *
     * Trying to apply a query that failed to compiled will fail and do nothing.
     *
     * You should call notifyQueriesApplied() to update the UI if true is
     * returned. applyQuery() will return true if successful, and applyAllQueries()
     * will return true if any of the queries was applied successfully.
     */
    bool applyQuery(int queryIdx);
    bool applyAllQueries();
    void notifyQueriesApplied();

    const rfcommon::Vector<Range>& matches(int queryIdx) const
        { return queryResults_[queryIdx].matches; }
    const rfcommon::Vector<Sequence>& mergedMatches(int queryIdx) const
        { return queryResults_[queryIdx].mergedMatches; }
    const rfcommon::Vector<Sequence>& mergedAndNormalizedMatches(int queryIdx) const
        { return queryResults_[queryIdx].mergedAndNormalizedMatches; }
    const rfcommon::Vector<Range>& sessionMatches(int queryIdx, int sessionIdx) const
        { return queryResults_[queryIdx].sessionMatches[sessionIdx]; }
    const rfcommon::Vector<Sequence>& sessionMergedMatches(int queryIdx, int sessionIdx) const
        { return queryResults_[queryIdx].sessionMergedMatches[sessionIdx]; }

    rfcommon::ListenerDispatcher<SequenceSearchListener> dispatcher;

private:
    const rfcommon::MotionLabels* const labels_;

    // Accumulates all states spanned over all sessions for every unique fighter
    rfcommon::Vector<States> fighterStates_;

    struct Session
    {
        // Time stamp started
        rfcommon::TimeStamp timeStarted;
        // Session specific ranges mapping into fighterStates_[x]
        // Vector is always the same size as fighterStates_.count(), but if
        // the fighter does not exist in this session then the range will
        // be empty
        rfcommon::Vector<Range> fighterStatesRange;
        rfcommon::String sessionName;
    };
    rfcommon::Vector<Session> sessions_;

    // Since we store data over multiple sessions, the index of a fighter in a
    // single session will not always correspond to the index of a fighter in
    // our "fighterStates_" member. This maps the session's fighter index to our
    // internal "fighterStates_" index.
    rfcommon::SmallVector<int, 8> fighterIdxMapFromSession_;

    // Current player + opponent indices, set by setPlayerPOV() and setOpponentPOV()
    int playerPOV_ = -1;
    int opponentPOV_ = -1;

    struct QueryStrings
    {
        rfcommon::String player;
        rfcommon::String opponent;
    };

    struct QueryNFAs
    {
        std::unique_ptr<Query> player;
        std::unique_ptr<Query> opponent;
    };

    struct QueryResult
    {
        rfcommon::Vector<Range> matches;
        rfcommon::Vector<Sequence> mergedMatches;
        rfcommon::Vector<Sequence> mergedAndNormalizedMatches;
        rfcommon::Vector<rfcommon::Vector<Range>> sessionMatches;
        rfcommon::Vector<rfcommon::Vector<Sequence>> sessionMergedMatches;
        rfcommon::Vector<rfcommon::Vector<Sequence>> sessionMergedAndNormalizedMatches;
    };

    rfcommon::Vector<QueryStrings> queryStrings_;
    rfcommon::Vector<QueryResult> queryResults_;
    rfcommon::Vector<QueryNFAs> compiledQueries_;

    rfcommon::FighterID previousFighterID_;
    rfcommon::String previousPlayerName_;
    rfcommon::FighterID previousOpponentID_;
    rfcommon::String previousOpponentName_;
};
