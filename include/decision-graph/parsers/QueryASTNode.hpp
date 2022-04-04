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
        QUALIFIER
    } type;

    enum QualifierFlags {
        QUAL_OS    = 0x01,
        QUAL_OOS   = 0x02,
        QUAL_HIT   = 0x04,
        QUAL_WHIFF = 0x08,
        QUAL_FH    = 0x10,
        QUAL_SH    = 0x20,
        QUAL_DJ    = 0x04,
        QUAL_IDJ   = 0x80
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

    struct Qualifier {
        Qualifier(QueryASTNode* child, uint8_t flags) : child(child), flags(flags) {}
        QueryASTNode* child;
        uint8_t flags;
    };

private:
    QueryASTNode(Statement statement) : type(STATEMENT), statement(statement)
        {}
    QueryASTNode(Repitition repitition) : type(REPITITION), repitition(repitition)
        {}
    QueryASTNode(Union union_) : type(UNION), union_(union_)
        {}
    QueryASTNode(Inversion inversion) : type(INVERSION), inversion(inversion)
        {}
    QueryASTNode(Type type) : type(type)
        {}
    QueryASTNode(const char* label) : type(LABEL), label(label)
        {}
    QueryASTNode(Qualifier qualifier) : type(QUALIFIER), qualifier(qualifier)
        {}
    ~QueryASTNode() {}

public:
    static QueryASTNode* newStatement(QueryASTNode* child, QueryASTNode* next);
    static QueryASTNode* newRepitition(QueryASTNode* child, int minreps, int maxreps);
    static QueryASTNode* newUnion(QueryASTNode* child, QueryASTNode* next);
    static QueryASTNode* newInversion(QueryASTNode* child);
    static QueryASTNode* newWildcard();
    static QueryASTNode* newLabel(const char* label);
    static QueryASTNode* newQualifier(QueryASTNode* child, uint8_t flags);

    static void destroySingle(QueryASTNode* node);
    static void destroyRecurse(QueryASTNode* node);

    void exportDOT(const char* filename) const;

    union {
        Statement statement;
        Repitition repitition;
        Union union_;
        Inversion inversion;
        rfcommon::SmallString<7> label;
        Qualifier qualifier;
    };
};
