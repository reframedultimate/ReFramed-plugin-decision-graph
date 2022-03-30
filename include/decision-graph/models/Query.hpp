#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"

class Matcher
{
public:
    enum MatchFlags
    {
        MATCH_MOTION = 0x01,
        MATCH_STATUS = 0x02,
    };

    enum HitType
    {
        HIT_CONNECT   = 0x01,
        HIT_WHIFF     = 0x02,
        HIT_ON_SHIELD = 0x04
    };

    //! Wildcard, matches anything
    static Matcher wildCard();

    //! Match a specific motion hash40 value. Hit type, status, and flags don't matter
    static Matcher motion(rfcommon::FighterMotion motion);

    bool matches(const State& node) const;

    rfcommon::Vector<int> next;

private:
    Matcher(rfcommon::FighterMotion motion, rfcommon::FighterStatus status, uint8_t hitType, uint8_t matchFlags);

    const rfcommon::FighterMotion motion_;
    const rfcommon::FighterStatus status_;
    const uint8_t hitType_;
    const uint8_t matchFlags_;
};

class Query
{
public:
    static Query nair_mixup_example();
    static Query nair_wildcard_example();
    rfcommon::Vector<SequenceRange> apply(const Sequence& sequence);

private:
    rfcommon::Vector<Matcher> matchers_;
};
