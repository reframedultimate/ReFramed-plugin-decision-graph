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
     * sessions and adopt the data from the newly added sessions.
     *
     * It's also possible to add frame data to each session you create before
     * calling notifyNewSessions(). This will cause the UI to adopt the new
     * data instead of showing empty data. It is not necessary to call
     * notifyFramesAdded(), as this is implied by notifyNewSessions().
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
     *
     * Note that notifyNewSessions() implies notifyFramesAdded().
     */
    void addFrame(int frameIdx, const rfcommon::FrameData* fdata);
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
    int fighterCount() const;
    int playerFighter() const;
    int opponentFighter() const;
    const char* playerName(int fighterIdx) const;
    const char* fighterName(int fighterIdx) const;
    rfcommon::FighterID fighterID(int fighterIdx) const;
    const States& fighterStates(int fighterIdx) const;

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
    int queryCount() const;
    const char* queryStr(int queryIdx) const;

    /*
     * Once the query strings are set, they need to be compiled. This needs to
     * be done whenever the search string changes, or if a fighter changes.
     *
     * This calls onQueryCompiled(). The success flag (and error message) are
     * passed to this callback. Additionally, if compilation succeeds, true is
     * returned.
     */
    bool compileQuery(int queryIdx, rfcommon::FighterID fighterID, rfcommon::FighterID oppFighterID);
    const char* lastQueryError();

    /*
     * Applies queries to the current data. Whenever frames are added, or new
     * sessions are added, or if a query is re-compiled, this should be called.
     *
     * Trying to apply a query that failed to compiled will fail and not do
     * anything.
     *
     * Calls onQueriesApplied()
     */
    bool applyQuery(int queryIdx);
    bool applyAllQueries();

    const rfcommon::Vector<Range>& matches(int queryIdx) const;
    const rfcommon::Vector<Sequence>& mergedMatches(int queryIdx) const;
    const rfcommon::Vector<Range>& sessionMatches(int queryIdx, int sessionIdx) const;
    const rfcommon::Vector<Sequence>& sessionMergedMatches(int queryIdx, int sessionIdx) const;

    rfcommon::ListenerDispatcher<SequenceSearchListener> dispatcher;

private:
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
    int currentFighterIdx_ = -1;
    int opponentFighterIdx_ = -1;

    struct QueryResult
    {
        Graph graph;
        rfcommon::Vector<Range> matches;
        rfcommon::Vector<Sequence> mergedMatches;
        rfcommon::Vector<Sequence> mergedAndNormalizedMatches;
        rfcommon::Vector<Graph> sessionGraph;
        rfcommon::Vector<rfcommon::Vector<Range>> sessionMatches;
        rfcommon::Vector<rfcommon::Vector<Sequence>> sessionMergedMatches;
        rfcommon::Vector<rfcommon::Vector<Sequence>> sessionMergedAndNormalizedMatches;
    };
    rfcommon::Vector<QueryResult> queryResults_;
    rfcommon::Vector<std::unique_ptr<Query>> queries_;
    rfcommon::Vector<rfcommon::String> queryStrings_;
    rfcommon::String queryError_;

    rfcommon::FighterID previousFighterID_;
    rfcommon::String previousPlayerName_;
};
