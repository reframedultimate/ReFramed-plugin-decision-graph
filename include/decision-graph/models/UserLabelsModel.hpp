#pragma once

#include "decision-graph/models/UserLabelCategory.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/ListenerDispatcher.hpp"

class UserLabelsListener;

class MotionLabels
{
public:
    struct Entry
    {
        Entry(rfcommon::FighterMotion motion, const rfcommon::SmallString<31>& label) : motion(motion), label(label) {}

        rfcommon::FighterMotion motion;
        rfcommon::SmallString<31> label;
    };

    rfcommon::Vector<Entry> entries;
    rfcommon::HashMap<rfcommon::FighterMotion, int, rfcommon::FighterMotion::Hasher> motionMap;
    rfcommon::HashMap<rfcommon::SmallString<31>, int> labelMap;
};

class FighterUserLabels
{
public:
    enum MatchFlags
    {
        MATCH_MOTION = 0x01,
        MATCH_STATUS = 0x02,
    };

    struct Entry
    {
        rfcommon::String userLabel;
        rfcommon::String statusName;
        rfcommon::FighterMotion motion;
        rfcommon::FighterStatus status;
        UserLabelCategory category;
        uint8_t matchFlags;
    };

    rfcommon::Vector<Entry> entries;
    rfcommon::HashMap<rfcommon::FighterMotion, int, rfcommon::FighterMotion::Hasher> motionMap;
    rfcommon::HashMap<rfcommon::String, rfcommon::SmallVector<int, 4>> userMap;
};

class UserLabelsModel
{
public:
    bool loadMotionLabels(const char* fileName);
    bool loadFighterUserLabels(const char* fileName);

    const char* motionToLabel(rfcommon::FighterMotion motion) const;
    const char* motionToUserLabel(rfcommon::FighterMotion motion, rfcommon::FighterID fighterID) const;
    rfcommon::SmallVector<rfcommon::FighterMotion, 4> userLabelToMotion(const char* userLabel, rfcommon::FighterID fighterID) const;
    rfcommon::FighterMotion labelToMotion(const char* label) const;

    rfcommon::ListenerDispatcher<UserLabelsListener> dispatcher;

private:
    MotionLabels motionLabels;
    rfcommon::Vector<FighterUserLabels> fighterUserLabels;
};
