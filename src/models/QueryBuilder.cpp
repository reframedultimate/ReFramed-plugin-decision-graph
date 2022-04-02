#include "decision-graph/models/QueryBuilder.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"
#include "decision-graph/parsers/QueryParser.y.hpp"
#include "decision-graph/parsers/QueryScanner.lex.hpp"
#include <cstring>

// ----------------------------------------------------------------------------
QueryBuilder::QueryBuilder(const MotionsTable* motionsTable)
    : motionsTable_(motionsTable)
{
}

// ----------------------------------------------------------------------------
bool QueryBuilder::parse(const char* text)
{
    qpscan_t scanner;
    qppstate* parser;
    YY_BUFFER_STATE buf;
    QPSTYPE pushed_value;
    int pushed_char;
    int parse_result;

    if (qplex_init_extra(this, &scanner) != 0)
        goto init_scanner_failed;
    buf = qp_scan_bytes(text, strlen(text), scanner);
    if (buf == nullptr)
        goto scan_bytes_failed;
    parser = qppstate_new();
    if (parser == nullptr)
        goto init_parser_failed;

    do
    {
        pushed_char = qplex(&pushed_value, scanner);
        parse_result = qppush_parse(parser, pushed_char, &pushed_value, scanner);
    } while (parse_result == YYPUSH_MORE);

    qppstate_delete(parser);
    qp_delete_buffer(buf, scanner);
    qplex_destroy(scanner);
    return parse_result == 0;

    init_parser_failed : qp_delete_buffer(buf, scanner);
    scan_bytes_failed: qplex_destroy(scanner); 
    init_scanner_failed : return false;
}

// ----------------------------------------------------------------------------
void QueryBuilder::parseAST(const QueryASTNode* ast)
{
    ast->exportDOT("query-ast.dot");
}
