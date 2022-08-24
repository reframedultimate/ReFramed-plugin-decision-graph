#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"

class LabelMapper;
class Query;
class SequenceRange;
struct QueryASTNode;

/*!
 * \brief Represents a single state in the NFA (non-deterministic finite automaton)
 */
class Matcher
{
public:
    enum MatchFlags
    {
        MATCH_ACCEPT = 0x01,
        MATCH_MOTION = 0x02,
        MATCH_STATUS = 0x04,
    };

    enum ContextQualifier
    {
        HIT       = 0x01,
        WHIFF     = 0x02,
        SHIELD    = 0x04,
    };

    /*!
     * \brief Creates the start state. This state does not match anything. This
     * is used as a "container" to store all of the initial matchers, since
     * it's possible that a NFA has more than 1 initial matcher.
     */
    static Matcher start();

    /*!
     * \brief Creates a wildcard which matches any hash40 (motion) or status value.
     * You can additionally specify the context in which these motion values occur.
     */
    static Matcher wildCard(uint8_t contextFlags);

    /*!
     * \brief Creates a state that only matches hash40 (motion) values. Hit
     * type, status, and fighter flags don't matter.
     */
    static Matcher motion(rfcommon::FighterMotion motion, uint8_t contextQualifierFlags);

    //! Set this matcher as the stop condition
    Matcher& setAcceptCondition()
        { matchFlags_ |= MATCH_ACCEPT; return *this; }

    bool isAcceptCondition() const
        { return !!(matchFlags_ & MATCH_ACCEPT); }

    bool isWildcard() const
        { return !(matchFlags_ & (MATCH_MOTION | MATCH_STATUS)); }

    bool inContext(ContextQualifier flag) { return !!(ctxQualFlags_ & flag); }

    bool matches(const State& node) const;

    rfcommon::Vector<int> next;

private:
    friend class Query;
    Matcher(
            rfcommon::FighterMotion motion,
            rfcommon::FighterStatus status,
            uint8_t hitType,
            uint8_t contextQualifierFlags);

    rfcommon::FighterMotion motion_;
    int motionLayerIdx_;
    rfcommon::FighterStatus status_;
    uint8_t ctxQualFlags_;
    uint8_t matchFlags_;
};

class Query
{
public:
    static QueryASTNode* parse(const char* text);
    static Query* compileAST(const QueryASTNode* ast, const LabelMapper* labels, rfcommon::FighterID fighterID);
    rfcommon::Vector<Sequence> apply(const States& states, const Sequence& seq);
    void exportDOT(const char* filename, const LabelMapper* labels, rfcommon::FighterID fighterID);

private:
    friend class QueryBuilder;
    rfcommon::Vector<Matcher> matchers_;
};
