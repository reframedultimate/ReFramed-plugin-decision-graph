#pragma once

#include "rfcommon/Reference.hpp"
#include "rfcommon/FighterID.hpp"
#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/String.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {
    class Hash40Strings;
    class UserMotionLabels;
}

class LabelMapper
{
public:
    LabelMapper(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings);
    ~LabelMapper();

    rfcommon::SmallVector<rfcommon::FighterMotion, 4> matchUserLabels(rfcommon::FighterID fighterID, const char* label) const;
    rfcommon::FighterMotion matchKnownHash40(const char* label) const;
    rfcommon::String bestEffortStringAllLayers(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion) const;
    rfcommon::String bestEffortStringHighestLayer(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion) const;
    rfcommon::String hash40StringOrHex(rfcommon::FighterMotion motion) const;

private:
    rfcommon::Reference<rfcommon::UserMotionLabels> userLabels_;
    rfcommon::Reference<rfcommon::Hash40Strings> hash40Strings_;
};
