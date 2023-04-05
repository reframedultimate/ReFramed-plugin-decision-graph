#pragma once

#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/Hashers.hpp"
#include "rfcommon/Vec2.hpp"

class State
{
public:
    struct SideData;

    enum InteractionQualifier
    {
        NO_INTERACTION,
        TRADE,
        ADVANTAGE,
        DISADVANTAGE,
        ON_SHIELD,
        SHIELD_LAG,
    };

    State(
            const SideData& sideData,
            rfcommon::FighterMotion motion,
            rfcommon::FighterStatus status,
            bool inHitlag, bool inHitstun, bool inShieldlag,
            bool oppInHitlag, bool oppInHitstun, bool oppInShieldlag)
        : motion(motion)
        , sideData(sideData)
        , status(status)
        , flags(makeFlags(inHitlag, inHitstun, inShieldlag, oppInHitlag, oppInHitstun, oppInShieldlag))
    {}

    static uint8_t makeFlags(
            bool inHitlag, bool inHitstun, bool inShieldlag,
            bool opponentInHitlag, bool opponentInHitstun, bool opponentInShieldlag)
    {
        return  (static_cast<uint8_t>(inHitlag) << 0)
              | (static_cast<uint8_t>(inHitstun) << 1)
              | (static_cast<uint8_t>(inShieldlag) << 2)
              | (static_cast<uint8_t>(opponentInHitlag) << 3)
              | (static_cast<uint8_t>(opponentInHitstun) << 4)
              | (static_cast<uint8_t>(opponentInShieldlag) << 5);
    }

    InteractionQualifier interaction() const
    {
        if ((inHitlag() || inHitstun()) && (opponentInHitlag() || opponentInHitstun()))
            return TRADE;
        if (inHitlag() || inHitstun())
            return DISADVANTAGE;
        if (opponentInHitlag() || opponentInHitstun())
            return ADVANTAGE;
        if (opponentInShieldlag())
            return ON_SHIELD;
        if (inShieldlag())
            return SHIELD_LAG;
        return NO_INTERACTION;
    }

    bool inHitlag() const { return !!(flags & 0x01); }
    bool inHitstun() const { return !!(flags & 0x02); }
    bool inShieldlag() const { return !!(flags & 0x04); }
    bool opponentInHitlag() const { return !!(flags & 0x08); }
    bool opponentInHitstun() const { return !!(flags & 0x10); }
    bool opponentInShieldlag() const { return !!(flags & 0x20); }

    // Data not relevant when comparing states
    struct SideData
    {
        SideData(
            rfcommon::FrameIndex frameIndex,
            rfcommon::Vec2 position,
            float damage,
            float shield)
            : position(position)
            , damage(damage)
            , shield(shield)
            , frameIndex(frameIndex)
        {}

        const rfcommon::Vec2 position;
        const float damage;
        const float shield;
        const rfcommon::FrameIndex frameIndex;
    };

    const rfcommon::FighterMotion motion;        // u64
    const SideData sideData;                     // f32, f32, f32, f32, u32
    const rfcommon::FighterStatus status;        // u16
    uint8_t flags;                               // u8

private:
    bool operator==(const State& other) const { return false; }
};
