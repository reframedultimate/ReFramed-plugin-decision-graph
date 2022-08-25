%require "3.8"
%code top
{
    class QueryASTNode;

    #include "decision-graph/parsers/QueryParser.y.hpp"
    #include "decision-graph/parsers/QueryScanner.lex.hpp"
    #include "decision-graph/parsers/QueryASTNode.hpp"
    #include "decision-graph/util/Str.hpp"

    static void addDamageRangeChild(QueryASTNode* root, QueryASTNode* child);
    static void qperror(qpscan_t scanner, const char* msg, ...);
}

%code requires
{
    #define YYSTYPE QPSTYPE
    #define YYLTYPE QPLTYPE

    #include <cstdint>
    #include <cstdarg>

    typedef void* qpscan_t;
    typedef struct qppstate qppstate;
}

/*
 * This is the bison equivalent of Flex's %option reentrant, in the sense that it also makes formerly global
 * variables into local ones. Unlike the lexer, there is no state structure for Bison. All the formerly global
 * variables become local to the yyparse() method. Which really begs the question: why were they ever global?
 * Although it is similar in nature to Flex's %option reentrant, this is truly the counterpart of
 * Flex's %option bison-bridge. Adding this declaration is what causes Bison to invoke yylval(YYSTYPE*) instead
 * of yylval(void), which is the same change that %option bison-bridge does in Flex.
 */
%define api.pure full

/*
 * As far as the grammar file goes, this is the only change needed to tell Bison to switch to a push-parser
 * interface instead of a pull-parser interface. Bison has the capability to generate both, but that is a
 * far more advanced case not covered here.
 */
%define api.push-pull push

%define api.prefix {qp}

/* Tell bison where and how it should include the generated header file */
%define api.header.include {"decision-graph/parsers/QueryParser.y.hpp"}

/*
 * These two options are related to Flex's %option reentrant, These options add an argument to
 * each of the yylex() call and the yyparse() method, respectively.
 * Since the %option reentrant in Flex added an argument to make the yylex(void) method into yylex(yyscan_t),
 * Bison must be told to pass that new argument when it invokes the lexer. This is what the %lex-param declaration does.
 * How Bison obtains an instance of yyscan_t is up to you, but the most sensible way is to pass it into the
 * yyparse(void) method, making the  new signature yyparse(yyscan_t). This is what the %parse-param does.
 */
%parse-param {QueryASTNode** ast}

%define api.token.prefix {TOK_}

/* This is the union that will become known as QPSTYPE in the generated code */
%union {
    char* string_value;
    int integer_value;
    uint8_t ctx_flags;
    struct QueryASTNode* node_value;
}

%destructor { StrFree($$); } <string_value>
%destructor { QueryASTNode::destroySingle($$); } <node_value>

%token '.' '*' '+' '?' '(' ')' '|' '!'
%token '>' '<' '-'
%token INTO
%token OS
%token OOS
%token HIT
%token WHIFF
%token SH
%token FH
%token DJ
%token IDJ
%token<integer_value> NUM
%token<integer_value> PERCENT
%token<string_value> LABEL

%type<node_value> stmnts qual_stmnt stmnt repitition union inversion label damages damage
%type<ctx_flags> pre_qual post_qual

%right '|'

%start query

%%
query
  : stmnts                     { *ast = $1; }
  ;
stmnts
  : stmnts INTO qual_stmnt     { $$ = QueryASTNode::newStatement($1, $3); }
  | qual_stmnt                 { $$ = $1; }
  ;
qual_stmnt
  : stmnt damages              { $$ = $2; addDamageRangeChild($2, $1); }
  | stmnt                      { $$ = $1; }
  ;
stmnt
  : pre_qual union post_qual   { $$ = QueryASTNode::newContextQualifier($2, $1 | $3); }
  | union post_qual            { $$ = QueryASTNode::newContextQualifier($1, $2); }
  | pre_qual union             { $$ = QueryASTNode::newContextQualifier($2, $1); }
  | union                      { $$ = $1; }
  ;
union
  : union '|' union            { $$ = QueryASTNode::newUnion($1, $3); }
  | repitition                 { $$ = $1; }
  ;
repitition
  : inversion '+'              { $$ = QueryASTNode::newRepitition($1, 1, -1); }
  | inversion '*'              { $$ = QueryASTNode::newRepitition($1, 0, -1); }
  | inversion '?'              { $$ = QueryASTNode::newRepitition($1, 0, 1); }
  | inversion NUM              { $$ = QueryASTNode::newRepitition($1, $2, $2); }
  | inversion NUM ',' NUM      { $$ = QueryASTNode::newRepitition($1, $2, $4); }
  | inversion NUM ',' '+'      { $$ = QueryASTNode::newRepitition($1, $2, -1); }
  | inversion NUM ',' '*'      { $$ = QueryASTNode::newRepitition($1, $2, -1); }
  | inversion                  { $$ = $1; }
  ;
inversion
  : '!' label                  { $$ = QueryASTNode::newInversion($2); }
  | label                      { $$ = $1; }
  | '.'                        { $$ = QueryASTNode::newWildcard(); }
  | '(' stmnts ')'             { $$ = $2; }
  ;
label
  : LABEL                      { $$ = QueryASTNode::newLabel($1); StrFree($1); }
  ;
pre_qual
  : pre_qual '|' pre_qual      { $$ = $1; $$ |= $3; }
  | '(' pre_qual ')'           { $$ = $2; }
  | FH                         { $$ = QueryASTNode::FH; }
  | SH                         { $$ = QueryASTNode::SH; }
  | DJ                         { $$ = QueryASTNode::DJ; }
  | IDJ                        { $$ = QueryASTNode::IDJ; }
  ;
post_qual
  : post_qual '|' post_qual    { $$ = $1; $$ |= $3; }
  | '(' post_qual ')'          { $$ = $2; }
  | OS                         { $$ = QueryASTNode::OS; }
  | OOS                        { $$ = QueryASTNode::OOS; }
  | HIT                        { $$ = QueryASTNode::HIT; }
  | WHIFF                      { $$ = QueryASTNode::WHIFF; }
  ;
damages
  : damages damage             { $$ = $2; $2->damageRange.child = $1; }
  | damage                     { $$ = $1; }
  ;
damage
  : PERCENT '-' PERCENT        { $$ = QueryASTNode::newDamageRange(nullptr, $1, $3); }
  | '>' PERCENT                { $$ = QueryASTNode::newDamageRange(nullptr, $2, 999); }
  | '<' PERCENT                { $$ = QueryASTNode::newDamageRange(nullptr, 0, $2); }
  ;
%%

static void addDamageRangeChild(QueryASTNode* root, QueryASTNode* child)
{
    assert(root->type == QueryASTNode::DAMAGE_RANGE);
    while (root->damageRange.child)
    {
        root = root->damageRange.child;
        assert(root->type == QueryASTNode::DAMAGE_RANGE);
    }

    root->damageRange.child = child;
}

static void qperror(qpscan_t scanner, const char* msg, ...)
{
}
