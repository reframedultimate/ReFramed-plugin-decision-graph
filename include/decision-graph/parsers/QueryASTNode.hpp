#pragma once

#include "rfcommon/String.hpp"
#include <new>

struct QueryASTNode
{
    enum Type
    {
        STATEMENT,
        REPITITION,
        UNION,
        INVERSION,
        WILDCARD,
        LABEL,
        CONTEXT_QUALIFIER,
        DAMAGE_RANGE
    } type;

    enum ContextQualifierFlags {
        OS    = 0x01,
        OOS   = 0x02,
        HIT   = 0x04,
        WHIFF = 0x08,
        FH    = 0x10,
        SH    = 0x20,
        DJ    = 0x40,
        IDJ   = 0x80
    };

    struct Statement {
        Statement(QueryASTNode* child, QueryASTNode* next) : child(child), next(next) {}
        QueryASTNode* child;
        QueryASTNode* next;
    };

    struct Repitition {
        Repitition(QueryASTNode* child, int minreps, int maxreps) : child(child), minreps(minreps), maxreps(maxreps) {}
        QueryASTNode* child;
        int minreps, maxreps;
    };

    struct Union {
        Union(QueryASTNode* child, QueryASTNode* next) : child(child), next(next) {}
        QueryASTNode* child;
        QueryASTNode* next;
    };

    struct Inversion {
        Inversion(QueryASTNode* child) : child(child) {}
        QueryASTNode* child;
    };

    struct ContextQualifier {
        ContextQualifier(QueryASTNode* child, uint8_t flags) : child(child), flags(flags) {}
        QueryASTNode* child;
        uint8_t flags;
    };

    struct DamageRange {
        DamageRange(QueryASTNode* child, int lower, int upper) : child(child), lower(lower), upper(upper) {}
        QueryASTNode* child;
        int lower, upper;
    };

private:
    QueryASTNode(Statement statement) : type(STATEMENT), statement(statement) {}
    QueryASTNode(Repitition repitition) : type(REPITITION), repitition(repitition) {}
    QueryASTNode(Union union_) : type(UNION), union_(union_) {}
    QueryASTNode(Inversion inversion) : type(INVERSION), inversion(inversion) {}
    QueryASTNode(Type type) : type(type) {}
    QueryASTNode(const char* label) : type(LABEL), label(label) {}
    QueryASTNode(ContextQualifier contextQualifier) : type(CONTEXT_QUALIFIER), contextQualifier(contextQualifier) {}
    QueryASTNode(DamageRange damageRange) : type(DAMAGE_RANGE), damageRange(damageRange) {}
    ~QueryASTNode() {}

public:
    static QueryASTNode* newStatement(QueryASTNode* child, QueryASTNode* next);
    static QueryASTNode* newRepitition(QueryASTNode* child, int minreps, int maxreps);
    static QueryASTNode* newUnion(QueryASTNode* child, QueryASTNode* next);
    static QueryASTNode* newInversion(QueryASTNode* child);
    static QueryASTNode* newWildcard();
    static QueryASTNode* newLabel(const char* label);
    static QueryASTNode* newContextQualifier(QueryASTNode* child, uint8_t contextQualifierFlags);
    static QueryASTNode* newDamageRange(QueryASTNode* child, int lower, int upper);

    static void destroySingle(QueryASTNode* node);
    static void destroyRecurse(QueryASTNode* node);

    void exportDOT(const char* filename) const;

    union {
        Statement statement;
        Repitition repitition;
        Union union_;
        Inversion inversion;
        rfcommon::SmallString<7> label;
        ContextQualifier contextQualifier;
        DamageRange damageRange;
    };
};
