#pragma once

class SequenceSearchListener
{
public:
    virtual void onCurrentFighterChanged() = 0;
    virtual void onNewSession() = 0;
    virtual void onDataAdded() = 0;
    virtual void onDataCleared() = 0;
    virtual void onQueryCompiled(int queryIdx) = 0;
    virtual void onQueryApplied() = 0;
};
