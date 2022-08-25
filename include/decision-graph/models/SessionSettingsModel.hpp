#pragma once

#include "rfcommon/ListenerDispatcher.hpp"

class SessionSettingsListener;

class SessionSettingsModel
{
public:
    SessionSettingsModel();
    ~SessionSettingsModel();

    void setAccumulateLiveSessions(bool enable);
    void clearPreviousSessions();

    bool accumulateLiveSessions() const 
            { return accumulateLiveSessions_; }

    rfcommon::ListenerDispatcher<SessionSettingsListener> dispatcher;

private:
    bool accumulateLiveSessions_ = true;
};