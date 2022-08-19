#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/Graph.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/FrameDataListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include "rfcommon/MetaDataListener.hpp"
#include <memory>

class Query;
class SequenceSearchListener;

namespace rfcommon {
    class FrameData;
    class MappingInfo;
    class MetaData;
}

class SequenceSearchModel
{
public:
    SequenceSearchModel(const LabelMapper* labelMapper);

    int sessionCount() const;
    void startNewSession(const rfcommon::MappingInfo* map, const rfcommon::MetaData* mdata);
    void addFrame(int frameIdx, const rfcommon::FrameData* fdata);
    void addAllFrames(const rfcommon::FrameData* fdata);
    void clearAll();

    int fighterCount() const;
    int currentFighter() const;
    void setCurrentFighter(int fighterIdx);
    const char* playerName(int fighterIdx) const;
    const char* fighterName(int fighterIdx) const;
    rfcommon::FighterID fighterID(int fighterIdx) const;

    int queryCount() const;
    void addQuery();
    void removeQuery(int queryIdx);
    bool setQuery(int queryIdx, const char* queryStr);
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
    const rfcommon::Vector<SequenceRange>& matches(int queryIdx) const;
    const Graph& sessionGraph(int queryIdx, int sessionIdx) const;
    const rfcommon::Vector<SequenceRange>& sessionMatches(int queryIdx, int sessionIdx) const;

    rfcommon::ListenerDispatcher<SequenceSearchListener> dispatcher;

private:
    void addFrameNoNotify(int frameIdx, const rfcommon::FrameData* fdata);
    bool applyQueryNoNotify(int queryIdx);

private:
    const LabelMapper* const labelMapper_;

    struct Session
    {
        // Time stamp started
        rfcommon::TimeStamp timeStarted;
        // Session specific ranges mapping into fighters_[x].sequence.
        // Vector is always the same size as fighters_.count(), but if
        // the fighter does not exist in this session then the range will
        // be empty
        rfcommon::Vector<SequenceRange> fighters;
    };
    rfcommon::Vector<Session> sessions_;

    struct Fighter
    {
        rfcommon::FighterID id;
        rfcommon::String playerName;
        rfcommon::String fighterName;
        rfcommon::Vector<SequenceRange> sessions;
        Sequence sequence;
    };
    rfcommon::Vector<Fighter> fighters_;
    rfcommon::SmallVector<int, 8> fighterIdxMapFromSession_;
    int currentFighterIdx_ = -1;

    struct QueryResult
    {
        Graph graph;
        rfcommon::Vector<SequenceRange> matches;
        rfcommon::Vector<Graph> sessionGraph;
        rfcommon::Vector<rfcommon::Vector<SequenceRange>> sessionMatches;
    };
    rfcommon::Vector<QueryResult> queryResults_;
    rfcommon::Vector<std::unique_ptr<Query>> queries_;
    rfcommon::Vector<rfcommon::String> queryStrings_;
    rfcommon::String queryError_;

    int frameCount_;
};
