#pragma once

#include "decision-graph/models/UserLabelCategory.hpp"
#include "rfcommon/Types.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/HashMap.hpp"
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

    struct FighterMotionHasher
    {
        typedef uint32_t HashType;
        HashType operator()(rfcommon::FighterMotion motion) const {
            return rfcommon::HashMapHasher<rfcommon::FighterMotion::Type, HashType>()(motion.value());
        }
    };

    rfcommon::Vector<Entry> entries;
    rfcommon::HashMap<rfcommon::FighterMotion, int, FighterMotionHasher> motionMap;
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

    struct FighterMotionHasher
    {
        typedef uint32_t HashType;
        HashType operator()(rfcommon::FighterMotion motion) const {
            return rfcommon::HashMapHasher<rfcommon::FighterMotion::Type, HashType>()(motion.value());
        }
    };

    rfcommon::Vector<Entry> entries;
    rfcommon::HashMap<rfcommon::FighterMotion, int, FighterMotionHasher> motionMap;
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
