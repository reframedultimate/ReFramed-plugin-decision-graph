%{
#define YYSTYPE QPSTYPE
#define YYLTYPE QPLTYPE

/*
#define YY_USER_ACTION \
    yylloc->first_line = yylloc->last_line; \
    yylloc->first_column = yylloc->last_column; \
    for(int i = 0; yytext[i] != '\0'; i++) { \
        if(yytext[i] == '\n') { \
            yylloc->last_line++; \
            yylloc->last_column = 1; \
        } \
        else { \
            yylloc->last_column++; \
        } \
    }*/

#include "decision-graph/parsers/QueryParser.y.hpp"
#include "decision-graph/util/Str.hpp"
#include <cstring>

%}

%option nodefault
%option noyywrap
%option bison-bridge
%option reentrant
%option prefix="qp"

%%
[\.\(\)\|\?\+\*!]       { return yytext[0]; }
-?>                     { return '>'; }
"os"                    { return TOK_OS; }
"oos"                   { return TOK_OOS; }
"hit"                   { return TOK_HIT; }
"whiff"                 { return TOK_WHIFF; }
"fh"                    { return TOK_FH; }
"sh"                    { return TOK_SH; }
"dj"                    { return TOK_DJ; }
"idj"                   { return TOK_IDJ; }
[0-9]+                  { yylval->integer_value = atoi(yytext); return TOK_NUM; }
[a-zA-Z_][a-zA-Z0-9_]+? { yylval->string_value = StrDup(yytext); return TOK_LABEL; }
" "
.                       { return -1; }
%%
