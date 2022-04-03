#include "decision-graph/models/QueryBuilder.hpp"
#include "decision-graph/models/MotionsTable.hpp"
#include "decision-graph/parsers/QueryASTNode.hpp"
#include "decision-graph/parsers/QueryParser.y.hpp"
#include "decision-graph/parsers/QueryScanner.lex.hpp"
#include "rfcommon/hash40.hpp"
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

    init_parser_failed  : qp_delete_buffer(buf, scanner);
    scan_bytes_failed   : qplex_destroy(scanner);
    init_scanner_failed : return false;
}

// ----------------------------------------------------------------------------
struct Fragment
{
    rfcommon::SmallVector<int, 4> in;
    rfcommon::SmallVector<int, 4> out;
};

// ----------------------------------------------------------------------------
static bool parseASTRecurse(
        const QueryASTNode* node,
        const MotionsTable* table,
        rfcommon::Vector<Matcher>* matchers,
        rfcommon::SmallVector<Fragment, 16>* stack)
{
    switch (node->type)
    {
    case QueryASTNode::STATEMENT: {
        if (!parseASTRecurse(node->statement.child, table, matchers, stack)) return false;
        if (!parseASTRecurse(node->statement.next, table, matchers, stack)) return false;
        if (stack->count() < 0) return false;

        Fragment& right = stack->back(1);
        Fragment& left = stack->back(2);
        for (int o : left.out)
            for (int i : right.in)
                matchers->at(o).next.push(i);
        left.out = right.in;
        stack->pop();
    } break;

    case QueryASTNode::REPITITION:
        if (!parseASTRecurse(node->repitition.child, table, matchers, stack)) return false;
        break;

    case QueryASTNode::UNION: {
        if (!parseASTRecurse(node->union_.child, table, matchers, stack)) return false;
        if (!parseASTRecurse(node->union_.next, table, matchers, stack)) return false;
        if (stack->count() < 2) return false;

        Fragment& f1 = stack->back(1);
        Fragment& f2 = stack->back(2);

        for (int i : f1.in)
            f2.in.push(i);
        for (int i : f1.out)
            f2.out.push(i);
        stack->pop();
    } break;

    case QueryASTNode::INVERSION:
        if (!parseASTRecurse(node->inversion.child, table, matchers, stack)) return false;
        break;

    case QueryASTNode::WILDCARD: {
        stack->push({{matchers->count()}, {matchers->count()}});
        matchers->push(Matcher::wildCard());
    } break;

    case QueryASTNode::LABEL: {
        // Assume label is a user label and maps to one more more motion
        // values
        auto motions = table->userLabelToMotion(node->label.cStr());
        if (motions.count() > 0)
        {
            Fragment& fragment = stack->emplace();
            for (const auto& motion : motions)
            {
                fragment.in.push(matchers->count());
                fragment.out.push(matchers->count());
                matchers->push(Matcher::motion(motion));
            }

            // If the user label maps to multiple motions a, b, c, then
            // connect them such that it matches [abc]+
            if (motions.count() > 1)
            {
                for (int a = 1; a <= motions.count(); ++a)
                    for (int b = 1; b <= motions.count(); ++b)
                        matchers->back(a).next.push(matchers->count() - b);
            }
            break;
        }

        // Assume label is actually a label and maps to a single motion
        // value
        auto motion = table->labelToMotion(node->label.cStr());
        if (motion.value() > 0)
        {
            stack->push({{matchers->count()}, {matchers->count()}});
            matchers->push(Matcher::motion(motion));
            break;
        }

        return false;
    } break;
    }

    return true;
}

// ----------------------------------------------------------------------------
bool QueryBuilder::parseAST(const QueryASTNode* ast)
{
    ast->exportDOT("query-ast.dot");

    query_.matchers_.clearCompact();
    query_.matchers_.push(Matcher::start());

    rfcommon::SmallVector<Fragment, 16> stack;
    if (!parseASTRecurse(ast, motionsTable_, &query_.matchers_, &stack))
        return false;
    if (stack.count() != 1)
        return false;

    for (int i : stack[0].in)
        query_.matchers_[0].next.push(i);

    query_.exportDOT("query.dot", motionsTable_);
    return true;
}
