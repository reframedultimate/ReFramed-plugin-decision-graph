#pragma once

class SequenceSearchListener
{
public:
    virtual void onNewSessions() = 0;
    virtual void onClearAll() = 0;
    virtual void onDataAdded() = 0;
    virtual void onPOVChanged() = 0;
    virtual void onQueryCompiled(int queryIdx) = 0;
    virtual void onQueriesApplied() = 0;
};
