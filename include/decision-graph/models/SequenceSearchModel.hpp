#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "decision-graph/models/Graph.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/Reference.hpp"
#include "rfcommon/SessionListener.hpp"
#include "rfcommon/ListenerDispatcher.hpp"
#include <memory>

namespace rfcommon {
    class Frame;
}

class Query;
class SequenceSearchListener;

class SequenceSearchModel : public rfcommon::SessionListener
{
public:
    SequenceSearchModel(const UserLabelsModel* userLabelsModel);

    void setSession(rfcommon::Session* session);
    void clearSession(rfcommon::Session* session);

    int fighterCount() const;
    const char* fighterName(int fighterIdx) const;
    const char* fighterCharacter(int fighterIdx) const;
    void setCurrentFighter(int fighterIdx);
    int currentFighter() const { return currentFighter_; }

    int frameCount() const;
    int sequenceLength(int fighterIdx) const;

    bool setQuery(const char* queryStr, int fighterIdx);
    const char* queryError() const { return queryError_.cStr(); }
    Graph applyQuery(int* numMatches, int* numMatchedStates);

    rfcommon::ListenerDispatcher<SequenceSearchListener> dispatcher;

private:
    void addFrame(int frameIdx, const rfcommon::Frame& frame);

private:
    // RunningGameSession events
    void onRunningGameSessionPlayerNameChanged(int fighterIdx, const rfcommon::SmallString<15>& name) override {}
    void onRunningGameSessionSetNumberChanged(rfcommon::SetNumber number) override {}
    void onRunningGameSessionGameNumberChanged(rfcommon::GameNumber number) override {}
    void onRunningGameSessionFormatChanged(const rfcommon::SetFormat& format) override {}
    void onRunningGameSessionWinnerChanged(int winnerPlayerIdx) override {}

    // RunningSession events
    void onRunningSessionNewUniqueFrame(int frameIdx, const rfcommon::Frame& frame) override;
    void onRunningSessionNewFrame(int frameIdx, const rfcommon::Frame& frame) override {}

private:
    const UserLabelsModel* userLabelsModel_;
    rfcommon::Reference<rfcommon::Session> session_;
    rfcommon::SmallVector<Sequence, 2> sequences_;
    std::unique_ptr<Query> query_;
    rfcommon::String queryError_;
    rfcommon::String currentFighterCharacter_;
    int currentFighter_ = 0;
};
