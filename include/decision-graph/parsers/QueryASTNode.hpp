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
        LABEL
    } type;

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

private:
    QueryASTNode(Statement statement) : type(STATEMENT), parent(nullptr), statement(statement)
        { statement.child->parent = this; statement.next->parent = this; }
    QueryASTNode(Repitition repitition) : type(REPITITION), parent(nullptr), repitition(repitition)
        { repitition.child->parent = this; }
    QueryASTNode(Union union_) : type(UNION), parent(nullptr), union_(union_)
        { union_.child->parent = this; union_.next->parent = this; }
    QueryASTNode(Inversion inversion) : type(INVERSION), parent(nullptr), inversion(inversion)
        { inversion.child->parent = this; }
    QueryASTNode(Type type) : type(type), parent(nullptr)
        {}
    QueryASTNode(const char* label) : type(LABEL), parent(nullptr), label(label)
        {}
    ~QueryASTNode() {}

public:
    static QueryASTNode* newStatement(QueryASTNode* child, QueryASTNode* next);
    static QueryASTNode* newRepitition(QueryASTNode* child, int minreps, int maxreps);
    static QueryASTNode* newUnion(QueryASTNode* child, QueryASTNode* next);
    static QueryASTNode* newInversion(QueryASTNode* child);
    static QueryASTNode* newWildcard();
    static QueryASTNode* newLabel(const char* label);

    static void destroySingle(QueryASTNode* node);
    static void destroyRecurse(QueryASTNode* node);

    void exportDOT(const char* filename) const;

    QueryASTNode* parent;
    union {
        Statement statement;
        Repitition repitition;
        Union union_;
        Inversion inversion;
        rfcommon::SmallString<7> label;
    };
};
