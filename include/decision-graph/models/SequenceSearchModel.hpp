#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/Graph.hpp"

#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetadataListener.hpp"
#include "rfcommon/Reference.hpp"
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
     * 
     */
    int sessionCount() const;
    const char* sessionName(int sessionIdx) const;
    void startNewSession(const rfcommon::MappingInfo* map, const rfcommon::Metadata* mdata);
    void addFrame(int frameIdx, const rfcommon::FrameData* fdata);
    void addAllFrames(const rfcommon::FrameData* fdata);
    void clearAll();

    int fighterCount() const;
    int currentFighter() const;
    void setCurrentFighter(int fighterIdx);
    const char* playerName(int fighterIdx) const;
    const char* fighterName(int fighterIdx) const;
    rfcommon::FighterID fighterID(int fighterIdx) const;
    const States& fighterStates(int fighterIdx) const;

    int queryCount() const;
    void addQuery();
    void removeQuery(int queryIdx);
    void setQuery(int queryIdx, const char* queryStr);
    void compileQuery(int queryIdx);
    bool queryCompiled(int queryIdx) const;
    const char* queryStr(int queryIdx) const;
    bool applyQuery(int queryIdx);
    bool applyAllQueries();
    const char* lastQueryError() const;

    int totalFrameCount() const;
    int totalSequenceLength() const;
    int totalMatchedSequences() const;
    int totalMatchedStates() const;
    int totalMatchedUniqueStates() const;

    const Graph& graph(int queryIdx) const;
    const rfcommon::Vector<Range>& matches(int queryIdx) const;
    const rfcommon::Vector<Sequence>& mergedMatches(int queryIdx) const;
    const Graph& sessionGraph(int queryIdx, int sessionIdx) const;
    const rfcommon::Vector<Range>& sessionMatches(int queryIdx, int sessionIdx) const;
    const rfcommon::Vector<Sequence>& sessionMergedMatches(int queryIdx, int sessionIdx) const;

    rfcommon::ListenerDispatcher<SequenceSearchListener> dispatcher;

private:
    void addFrameNoNotify(int frameIdx, const rfcommon::FrameData* fdata);
    bool applyQueryNoNotify(int queryIdx);

private:
    const rfcommon::MotionLabels* const labels_;

    struct Session
    {
        // Time stamp started
        rfcommon::TimeStamp timeStarted;
        // Session specific ranges mapping into fighters_[x].states.
        // Vector is always the same size as fighters_.count(), but if
        // the fighter does not exist in this session then the range will
        // be empty
        rfcommon::Vector<Range> fighters;
        rfcommon::String sessionName_;
    };
    rfcommon::Vector<Session> sessions_;

    rfcommon::Vector<States> fighters_;
    rfcommon::SmallVector<int, 8> fighterIdxMapFromSession_;
    int currentFighterIdx_ = -1;

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

    int frameCount_;
};
