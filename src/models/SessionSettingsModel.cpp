#include "decision-graph/models/SessionSettingsModel.hpp"
#include "decision-graph/listeners/SessionSettingsListener.hpp"

// ----------------------------------------------------------------------------
SessionSettingsModel::SessionSettingsModel()
{}

// ----------------------------------------------------------------------------
SessionSettingsModel::~SessionSettingsModel()
{}

// ----------------------------------------------------------------------------
void SessionSettingsModel::setAccumulateLiveSessions(bool enable)
{
    bool notify = (accumulateLiveSessions_ != enable);
    accumulateLiveSessions_ = enable;

    if (notify)
        dispatcher.dispatch(&SessionSettingsListener::onSessionSettingsChanged);
}

// ----------------------------------------------------------------------------
void SessionSettingsModel::clearPreviousSessions()
{
    dispatcher.dispatch(&SessionSettingsListener::onClearPreviousSessions);
}
