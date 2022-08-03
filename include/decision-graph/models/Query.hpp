#pragma once

#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"

class MappingInfo;
class UserLabelsModel;
class Query;
struct QueryASTNode;

class Matcher
{
public:
    enum MatchFlags
    {
        MATCH_STOP   = 0x01,
        MATCH_MOTION = 0x02,
        MATCH_STATUS = 0x04,
    };

    enum HitType
    {
        HIT       = 0x01,
        WHIFF     = 0x02,
        ON_SHIELD = 0x04,
    };

    static Matcher start();

    //! Set this matcher as the stop condition
    Matcher& setStop()
        { matchFlags_ |= MATCH_STOP; return *this; }

    bool isStop() const
        { return !!(matchFlags_ & MATCH_STOP); }

    //! Wildcard, matches anything
    static Matcher wildCard(uint8_t hitFlags);

    bool isWildcard() const
        { return !(matchFlags_ & (MATCH_MOTION | MATCH_STATUS)); }

    //! Match a specific motion hash40 value. Hit type, status, and flags don't matter
    static Matcher motion(rfcommon::FighterMotion motion, uint8_t hitFlags);

    bool hitFlagSet(HitType flag) { return !!(hitFlags_ & flag); }

    bool matches(const State& node) const;

    rfcommon::Vector<int> next;

private:
    friend class Query;
    Matcher(rfcommon::FighterMotion motion, rfcommon::FighterStatus status, uint8_t hitType, uint8_t matchFlags);

    rfcommon::FighterMotion motion_;
    rfcommon::FighterStatus status_;
    uint8_t hitFlags_;
    uint8_t matchFlags_;
};

class Query
{
public:
    static QueryASTNode* parse(const char* text);
    static Query* compileAST(const QueryASTNode* ast, const UserLabelsModel* table, rfcommon::FighterID fighterID);
    rfcommon::Vector<SequenceRange> apply(const Sequence& sequence);
    void exportDOT(const char* filename, const UserLabelsModel* table, rfcommon::FighterID fighterID);

private:
    friend class QueryBuilder;
    rfcommon::Vector<Matcher> matchers_;
};
