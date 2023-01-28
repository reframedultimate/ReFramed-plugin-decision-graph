#pragma once

#include "rfcommon/FighterMotion.hpp"
#include "rfcommon/FighterStatus.hpp"
#include "rfcommon/FighterHitStatus.hpp"
#include "rfcommon/FrameIndex.hpp"
#include "rfcommon/Hashers.hpp"
#include "rfcommon/Vec2.hpp"

class State
{
public:
    struct SideData;

    State(
            const SideData& sideData,
            rfcommon::FighterMotion motion,
            rfcommon::FighterStatus status,
            bool inHitlag, bool inHitstun, bool inShieldlag,
            bool opponentInHitlag, bool opponentInHitstun,
            bool opponentInShieldlag)
        : motion(motion)
        , sideData(sideData)
        , status(status)
        , flags(makeFlags(inHitlag, inHitstun, inShieldlag, opponentInHitlag, opponentInHitstun, opponentInShieldlag))
    {}

    // We only hash motion, status and flags, and NOT timings/position/shield/damage,
    // because for the purpose of searching for unique states and building graphs,
    // those values are irrelevant
    struct HasherNoSideData
    {
        typedef uint32_t HashType;
        HashType operator()(const State& node) const {
            const uint32_t motion_l = node.motion.lower();
            const uint16_t status = node.status.value();
            const uint8_t motion_h = node.motion.upper();
            const uint8_t flags = node.flags;

            const uint32_t a = motion_l;
            const uint32_t b = (status << 16) | (motion_h << 8) | (flags << 0);
            return rfcommon::hash32_combine(a, b);
        }
    };

    struct CompareNoSideData
    {
        bool operator()(const State& a, const State& b) const {
            return
                a.motion == b.motion &&
                a.status == b.status &&
                a.flags == b.flags;
        }
    };

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
