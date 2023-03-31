#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"
#include <cstdint>

class LabelMapper;
class Query;
class Range;
struct QueryASTNode;

/*!
 * \brief Represents a single state in the NFA (non-deterministic finite automaton)
 */
class Matcher
{
public:
    enum MatchFlags
    {
        MATCH_ACCEPT = 0x01,  // Is an accept condition
        MATCH_MOTION = 0x02,  // Match the motion hash40 value
        MATCH_STATUS = 0x04,  // Match the status value
    };

    enum ContextQualifier
    {
        HIT       = 0x01,  // Move connected with the opponent
        WHIFF     = 0x02,  // Move did not connect with the opponent or shield
        SHIELD    = 0x04,  // Move connected with shield
        RISING    = 0x08,  // Move is rising (y1 > y0)
        FALLING   = 0x10   // Move is falling (y1 < y0)
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
    static Matcher wildCard(uint8_t ctxQualFlags);

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

    rfcommon::Vector<int, int16_t> next;

private:
    friend class Query;
    Matcher(
            rfcommon::FighterMotion motion,
            rfcommon::FighterStatus status,
            uint8_t ctxQualFlags,
            uint8_t matchFlags);

    rfcommon::FighterMotion motion_;
    rfcommon::FighterStatus status_;
    uint8_t ctxQualFlags_;
    uint8_t matchFlags_;
};

class Query
{
public:
    /*!
     * \brief Parse a string into an AST
     * \param[in] text A string to parse.
     * \return Returns the root node of the AST if successful. The node must
     * be freed using QueryASTNode::destroyRecurse(). If parsing fails then
     * nullptr is returned.
     */
    static QueryASTNode* parse(const rfcommon::String& text);

    /*!
     * \brief Compiles an AST into a NFA, which can then be executed to find
     * matches in a sequence of fighter states.
     * \param[in] ast Root node of the AST as returned by parse().
     * \param[in] labels Motion labels table.
     * \param[in] fighterID Which fighter this AST should be compiled for. This
     * is necessary because each fighter may have different terminology and
     * meaning, so labels will compile differently depending on which fighter
     * it is intended for.
     * \param[out] error If an error occurs then more information is written
     * to this string.
     * \return Returns the compiled query if successful. Use "delete" to delete
     * the query later. The AST can be freed after compilation.
     */
    static Query* compileAST(const QueryASTNode* ast, const rfcommon::MotionLabels* labels, rfcommon::FighterID fighterID, rfcommon::String* error);

    /*!
     * \brief Finds the first match within the specified range.
     * \param[in] states The fighter states array to search.
     * \param[in] range The range within the fighter states array to search.
     * \return Returns [start,end) of the matched range. If no match was found
     * then range.startIdx == range.endIdx.
     */
    Range findFirst(const States& states, const Range& range) const;

    /*!
     * \brief Same as findFirst(), but it finds all matches within the specified range.
     */
    rfcommon::Vector<Range> findAll(const States& states, const Range& range) const;

    /*!
     * \brief Applies queries to two different state arrays at the same time,
     * and collects all ranges that overlap each other in time. This us used
     * when searching for player and opponent interactions.
     * \param otherQuery The query of the opponent. This gets applied to "otherStates"
     * and "otherRange".
     * \param states The state list of the player.
     * \param range The range within the state list of the player to search.
     * \param otherStates The state list of the opponent.
     * \param otherRange The range within the state list of the opponent to search.
     * \return When two ranges overlap, their intersection is
     */
    rfcommon::Vector<Range> findAllOverlapping(
            const Query* otherQuery,
            const States& states, const Range& range,
            const States& otherStates, const Range& otherRange) const;

    rfcommon::Vector<Sequence> mergeMotions(const States& states, const rfcommon::Vector<Range>& matches) const;
    rfcommon::Vector<Sequence> normalizeMotions(const States& states, const rfcommon::Vector<Sequence>& matches) const;
    void exportDOT(const char* filename, const rfcommon::MotionLabels* labels, rfcommon::FighterID fighterID);

private:
    friend class QueryBuilder;
    rfcommon::Vector<Matcher> matchers_;
    rfcommon::Vector<rfcommon::SmallVector<rfcommon::FighterMotion, 4>> mergeMotions_;
};
