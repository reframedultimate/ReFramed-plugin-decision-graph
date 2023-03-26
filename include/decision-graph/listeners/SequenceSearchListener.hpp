#pragma once

class SequenceSearchListener
{
public:
    virtual void onNewSessions() = 0;
    virtual void onClearAll() = 0;
    virtual void onDataAdded() = 0;
    virtual void onPOVChanged() = 0;
    virtual void onQueriesChanged() = 0;
    virtual void onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) = 0;
    virtual void onQueriesApplied() = 0;
};
