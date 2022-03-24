#pragma once

#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"
#include "rfcommon/FighterState.hpp"

namespace rfcommon {
    class Frame;
}

class Node
{
public:
    Node(
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

class NodeHash
{
public:
    NodeHash(const Node& node)
        : hashValue_([&node]() {
            const uint32_t motion_l = node.motion().lower();
            const uint16_t status = node.status().value();
            const uint8_t motion_h = node.motion().upper();
            const uint8_t hitStatus = node.hitStatus().value();
            const uint8_t flags = node.flags();

            const uint32_t a = motion_l;
            const uint32_t b = (status << 16) | (motion_h << 8) | (hitStatus << 0);
            const uint32_t c = flags;
            return rfcommon::hash32_combine(rfcommon::hash32_combine(a, b), c);
        }())
    {
    }

    uint32_t value() const
        { return hashValue_; }

    bool operator==(const NodeHash& other) const
        { return hashValue_ == other.hashValue_; }

    struct Hasher {
        typedef rfcommon::HashMapHasher<NodeHash>::HashType HashType;
        HashType operator()(const NodeHash& nodeHash) const {
            return nodeHash.value();
        }
    };

private:
    const uint32_t hashValue_;
};

class Edge
{
public:
    Edge(int from, int to)
        : from(from), to(to)
    {}

    int from, to;
};

class DecisionGraph
{
public:
    void clear();
    void addState(int fighterIdx, const rfcommon::Frame& frame);

    void addEdge(int from, int to);

    int numNodes() const { return nodes_.count(); }
    int numEdges() const { return edges_.count(); }

private:
    rfcommon::HashMap<NodeHash, int, NodeHash::Hasher> nodeLookup_;
    rfcommon::Vector<Node> nodes_;
    rfcommon::Vector<Edge> edges_;
    int prevNodeIdx_ = -1;
};
