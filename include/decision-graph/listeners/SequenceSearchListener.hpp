#pragma once

class SequenceSearchListener
{
public:
    virtual void onCurrentFighterChanged() = 0;
    virtual void onSessionChanged() = 0;
    virtual void onSequenceChanged() = 0;
    virtual void onQueryChanged() = 0;
};
