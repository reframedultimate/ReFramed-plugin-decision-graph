#pragma once

#include "decision-graph/models/DecisionGraph.hpp"
#include "rfcommon/Vector.hpp"

class NodeMatcher
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
    static NodeMatcher wildCard();

    //! Match a specific motion hash40 value. Hit type, status, and flags don't matter
    static NodeMatcher motion(rfcommon::FighterMotion motion);

    bool matches(const Node& node) const;

    rfcommon::Vector<int> next;

private:
    NodeMatcher(rfcommon::FighterMotion motion, rfcommon::FighterStatus status, uint8_t hitType, uint8_t matchFlags);

    const rfcommon::FighterMotion motion_;
    const rfcommon::FighterStatus status_;
    const uint8_t hitType_;
    const uint8_t matchFlags_;
};

class GraphQuery
{
public:
    static GraphQuery nair_utilt_example();
    DecisionGraph apply(const DecisionGraph& graph);

private:
    bool applyRecurse(
            const DecisionGraph& graph,
            int nodeIdx,
            int matchIdx,
            rfcommon::HashMap<Node, int, Node::Hasher>* foundNodes,
            rfcommon::HashMap<Node, int, Node::Hasher>* visited);

private:
    rfcommon::Vector<NodeMatcher> matchers_;
};
