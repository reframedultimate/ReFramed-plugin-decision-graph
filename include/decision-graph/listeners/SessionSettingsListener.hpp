#pragma once

class SessionSettingsListener
{
public:
    virtual void onSessionSettingsChanged() = 0;
    virtual void onClearPreviousSessions() = 0;
};
