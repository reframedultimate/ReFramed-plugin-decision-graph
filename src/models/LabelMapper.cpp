#include "decision-graph/models/LabelMapper.hpp"

#include "rfcommon/Hash40Strings.hpp"
#include "rfcommon/UserMotionLabels.hpp"
#include "rfcommon/hash40.hpp"

// ----------------------------------------------------------------------------
LabelMapper::LabelMapper(rfcommon::UserMotionLabels* userLabels, rfcommon::Hash40Strings* hash40Strings)
    : userLabels_(userLabels)
    , hash40Strings_(hash40Strings)
{}

// ----------------------------------------------------------------------------
LabelMapper::~LabelMapper()
{}

// ----------------------------------------------------------------------------
rfcommon::SmallVector<rfcommon::FighterMotion, 4> LabelMapper::matchUserLabels(rfcommon::FighterID fighterID, const char* label) const
{
    return userLabels_->toMotion(fighterID, label);
}

// ----------------------------------------------------------------------------
rfcommon::FighterMotion LabelMapper::matchKnownHash40(const char* label) const
{
    return hash40Strings_->toMotion(label);
}

// ----------------------------------------------------------------------------
rfcommon::String LabelMapper::bestEffortString(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion) const
{
    if (const char* label = userLabels_->toUserLabel(fighterID, motion, nullptr))
        return label;
    
    if (const char* str = hash40Strings_->toString(motion, nullptr))
        return str;

    char buf[13];
    static const char* digits = "0123456789ABCDEF";
    rfcommon::FighterMotion::Type value = motion.value();
    for (int i = 11; i >= 2; i--)  // hash40 value is 40 bits, or 5 bytes, or 10 nibbles
    {
        buf[i] = digits[(value & 0x0F)];
        value >>= 4;
    }
    buf[0] = '0';
    buf[1] = 'x';
    buf[7] = '\0';
    return buf;
}
