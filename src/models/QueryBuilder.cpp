#include "decision-graph/models/QueryBuilder.hpp"
#include <cstring>

// ----------------------------------------------------------------------------
static bool isNumber(char c)
{
    return c >= '0' && c <= '9';
}

// ----------------------------------------------------------------------------
static bool isAlpha(char c)
{
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z');
}

// ----------------------------------------------------------------------------
static bool isWhitespace(char c)
{
    return c == ' ';
}

// ----------------------------------------------------------------------------
static bool isAlphaNum(char c)
{
    return isNumber(c) || isAlpha(c);
}

// ----------------------------------------------------------------------------
static int toNumber(const char* begin, const char* end)
{
    int value = 0;
    while (begin != end)
    {
        value *= 10;
        value += *begin - '0';
        begin++;
    }
    return value;
}

// ----------------------------------------------------------------------------
QueryBuilder::QueryBuilder(const MotionsTable* motionsTable)
    : motionsTable_(motionsTable)
    , input_(nullptr)
{
}

// ----------------------------------------------------------------------------
bool QueryBuilder::parse(const char* text)
{
    newInput(text);

    // stmnts
    //   : stmnts
    //   | qualified_stmnt
    //   ;
    // qualified_stmnt
    //   : '(' stmnts ')'
    //   | qualified_stmnt '|' qualified_stmnt
    //   | qualified_stmnt "->" qualified_stmnt
    //   | stmnt
    //   ;
    // qualifier
    //   : '(' qualifier ')
    //   | qualifier '|' qualifier
    //   | "os"
    //   | "oos"
    //   | "hit"
    //   | "whiff"
    //   | "dmg"
    //   ;
    // stmnt
    //   : '.'
    //   | "die"
    //   | stmnt '+'
    //   | stmnt "os"
    //   | stmnt "oos"
    //   | stmnt "hit"
    //   | "whiff"
    //   | "die"
    //   | "dmg"
    //   | "label"
    //   ;

    int result = expectStmnt();
    for (; result > 0; result = expectStmnt())
    {

    }
}

// ----------------------------------------------------------------------------
int QueryBuilder::expectStmnt()
{
    expect
}

// ----------------------------------------------------------------------------
void QueryBuilder::newInput(const char* text)
{
    token_.begin = text;
    token_.end = text;
    query_.matchers_.clear();
}

// ----------------------------------------------------------------------------
int QueryBuilder::nextToken()
{
    if (*token_.end == '\0')
        return 0;

    if (isWhitespace(*token_.end))
        while (isWhitespace(*token_.end))
            token_.end++;

    token_.begin = token_.end;
    switch (*token_.end++)
    {
        case '.': {
            token_.type = Token::WILDCARD;
            token_.wildcardRepeats = 1;
            if (isNumber(*token_.end))
            {
                const char* begin = token_.end;
                while (isNumber(*token_.end))
                    token_.end++;
                token_.wildcardRepeats = toNumber(begin, token_.end);
            }
        } break;

        case '-':
            if (*token_.end != '>')
                return -1;
            token_.end++;
        case '>': {
            token_.type = Token::SEQDELIM;
        } break;

        case '(': {
            token_.type = Token::LPAREN;
        } break;

        case ')': {
            token_.type = Token::LPAREN;
        } break;

        case '|': {
            token_.type = Token::OR;
        } break;

        case '?': {
            token_.type = Token::OPTIONAL;
        } break;

        case '+': {
            token_.type = Token::REPEAT;
        } break;

        default: {
            if (strcmp(token_.end-1, "os") == 0)
            {
                token_.type = Token::ONSHIELD;
                token_.end += 1;
            }
            else if (strcmp(token_.end-1, "oos") == 0)
            {
                token_.type = Token::OUTOFSHIELD;
                token_.end += 2;
            }
            else if (strcmp(token_.end-1, "hit") == 0)
            {
                token_.type = Token::HIT;
                token_.end += 2;
            }
            else if (strcmp(token_.end-1, "whiff") == 0)
            {
                token_.type = Token::WHIFF;
                token_.end += 4;
            }
            else if (strcmp(token_.end-1, "die") == 0)
            {
                token_.type = Token::WHIFF;
                token_.end += 2;
            }
            else if (strcmp(token_.end-1, "dmg") == 0)
            {
                token_.type = Token::DMG;
                token_.end += 2;
            }
            else
            {
                token_.type = Token::LABEL;
                while (isAlphaNum(*token_.end))
                    token_.end++;
            }
        } break;
    }

    return 1;
}
