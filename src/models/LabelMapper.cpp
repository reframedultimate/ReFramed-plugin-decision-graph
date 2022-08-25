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
rfcommon::String LabelMapper::bestEffortStringAllLayers(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion) const
{
    const rfcommon::String label = userLabels_->toStringAllLayers(fighterID, motion, "");
    if (label.length())
        return label;

    return hash40StringOrHex(motion);
}

// ----------------------------------------------------------------------------
rfcommon::String LabelMapper::bestEffortStringHighestLayer(rfcommon::FighterID fighterID, rfcommon::FighterMotion motion) const
{
    const rfcommon::String label = userLabels_->toStringHighestLayer(fighterID, motion, "");
    if (label.length())
        return label;

    return hash40StringOrHex(motion);
}

// ----------------------------------------------------------------------------
rfcommon::String LabelMapper::hash40StringOrHex(rfcommon::FighterMotion motion) const
{
    if (const char* h40 = hash40Strings_->toString(motion, nullptr))
        return h40;

    char buf[13];
    static const char* digits = "0123456789abcdef";
    rfcommon::FighterMotion::Type value = motion.value();
    for (int i = 11; i >= 2; i--)  // hash40 value is 40 bits, or 5 bytes, or 10 nibbles
    {
        buf[i] = digits[(value & 0x0F)];
        value >>= 4;
    }
    buf[0] = '0';
    buf[1] = 'x';
    buf[12] = '\0';
    return buf;
}
