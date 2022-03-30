#pragma once

#include "rfcommon/Types.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/HashMap.hpp"

class MotionsTable
{
public:
    static MotionsTable load();

private:
    MotionsTable() {}

    struct Entry
    {
        Entry(rfcommon::FighterMotion motion, const rfcommon::SmallString<31>& label) : motion(motion), label(label) {}

        rfcommon::FighterMotion motion;
        rfcommon::SmallString<31> label;
        rfcommon::SmallString<7> user;
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
    rfcommon::HashMap<rfcommon::SmallString<7>, rfcommon::SmallVector<int, 4>> userMap;
};
