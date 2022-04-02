#pragma once

#include "decision-graph/models/Query.hpp"

class MotionsTable;
struct QueryASTNode;

class QueryBuilder
{
public:
    QueryBuilder(const MotionsTable* motionsTable);

    bool parse(const char* text);
    void parseAST(const QueryASTNode* ast);

private:
    const MotionsTable* motionsTable_;
    Query query_;
};
