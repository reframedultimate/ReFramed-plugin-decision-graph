#pragma once

#include "rfcommon/Types.hpp"

class State
{
    State(
            rfcommon::FighterMotion motion,
            rfcommon::FighterStatus status,
            rfcommon::FighterHitStatus hitStatus,
            uint8_t flags)
        : motion_(motion)
        , status_(status)
        , hitStatus_(hitStatus)
        , flags_(flags)
    {}

public:
    State(
            rfcommon::FighterMotion motion,
            rfcommon::FighterStatus status,
            rfcommon::FighterHitStatus hitStatus,
            bool inHitlag, bool inHitstun,
            bool opponentInHitlag, bool opponentInHitstun)
        : motion_(motion)
        , status_(status)
        , hitStatus_(hitStatus)
        , flags_(
              (static_cast<uint8_t>(inHitlag) << 0)
            | (static_cast<uint8_t>(inHitstun) << 1)
            | (static_cast<uint8_t>(opponentInHitlag) << 2)
            | (static_cast<uint8_t>(opponentInHitstun) << 3))
    {}

    struct Hasher
    {
        typedef rfcommon::HashMapHasher<State>::HashType HashType;
        HashType operator()(const State& node) const {
            const uint32_t motion_l = node.motion().lower();
            const uint16_t status = node.status().value();
            const uint8_t motion_h = node.motion().upper();
            const uint8_t hitStatus = node.hitStatus().value();
            const uint8_t flags = node.flags();

            const uint32_t a = motion_l;
            const uint32_t b = (status << 16) | (motion_h << 8) | (hitStatus << 0);
            const uint32_t c = flags;
            return rfcommon::hash32_combine(rfcommon::hash32_combine(a, b), c);
        }
    };

    bool operator==(const State& other) const
    {
        return motion_ == other.motion_ &&
                status_ == other.status_ &&
                hitStatus_ == other.hitStatus_ &&
                flags_ == other.flags_;
    }

    rfcommon::FighterMotion motion() const { return motion_; }
    rfcommon::FighterStatus status() const { return status_; }
    rfcommon::FighterHitStatus hitStatus() const { return hitStatus_; }
    uint8_t flags() const { return flags_; }

    bool inHitlag() const { return !!(flags_ & 0x01); }
    bool inHitstun() const { return !!(flags_ & 0x02); }
    bool opponentInHitlag() const { return !!(flags_ & 0x04); }
    bool opponentInHitstun() const { return !!(flags_ & 0x08); }

private:
    rfcommon::FighterMotion motion_;
    rfcommon::FighterStatus status_;
    rfcommon::FighterHitStatus hitStatus_;
    uint8_t flags_;
};
