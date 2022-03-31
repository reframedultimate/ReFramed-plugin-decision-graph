#pragma once

#include "decision-graph/models/Query.hpp"

class MotionsTable;

class QueryBuilder
{
public:
    struct Token
    {
        enum Type
        {
            WILDCARD,
            LPAREN,
            RPAREN,
            OPTIONAL,
            OR,
            REPEAT,
            SEQDELIM,
            ONSHIELD,
            OUTOFSHIELD,
            HIT,
            WHIFF,
            DIE,
            DMG,
            LABEL
        } type;

        const char* begin = nullptr;
        const char* end = nullptr;
        int wildcardRepeats = 1;
    };

    QueryBuilder(const MotionsTable* motionsTable);

    QueryBuilder& start();
    QueryBuilder& end();
    QueryBuilder& pushOptional();
    QueryBuilder& popOptional();
    QueryBuilder& pushOr();
    QueryBuilder& popOr();
    QueryBuilder& pushRepeat();
    QueryBuilder& popRepeat();
    QueryBuilder& pushQualifiers();
    QueryBuilder& popQualifiers();

    bool parse(const char* text);

    void newInput(const char* text);
    int nextToken();
    Token token() const { return token_; }

private:
    int expectStmnt();

private:
    const MotionsTable* motionsTable_;
    const char* input_;
    Token token_;
    Query query_;
};
