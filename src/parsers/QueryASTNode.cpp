#include "decision-graph/parsers/QueryASTNode.hpp"
#include "rfcommon/HashMap.hpp"
#include <cstdio>

// ----------------------------------------------------------------------------
QueryASTNode* QueryASTNode::newStatement(QueryASTNode* child, QueryASTNode* next)
{
    return new QueryASTNode(Statement(child, next));
}

// ----------------------------------------------------------------------------
QueryASTNode* QueryASTNode::newRepitition(QueryASTNode* child, int minreps, int maxreps)
{
    return new QueryASTNode(Repitition(child, minreps, maxreps));
}

// ----------------------------------------------------------------------------
QueryASTNode* QueryASTNode::newUnion(QueryASTNode* child, QueryASTNode* next)
{
    return new QueryASTNode(Union(child, next));
}

// ----------------------------------------------------------------------------
QueryASTNode* QueryASTNode::newInversion(QueryASTNode* child)
{
    return new QueryASTNode(Inversion(child));
}

// ----------------------------------------------------------------------------
QueryASTNode* QueryASTNode::newWildcard()
{
    return new QueryASTNode(WILDCARD);
}

// ----------------------------------------------------------------------------
QueryASTNode* QueryASTNode::newLabel(const char* label)
{
    return new QueryASTNode(label);
}

// ----------------------------------------------------------------------------
QueryASTNode* QueryASTNode::newContextQualifier(QueryASTNode* child, uint8_t contextQualifierFlags)
{
    return new QueryASTNode(ContextQualifier(child, contextQualifierFlags));
}

// ----------------------------------------------------------------------------
QueryASTNode* QueryASTNode::newDamageRange(QueryASTNode* child, int lower, int upper)
{
    return new QueryASTNode(DamageRange(child, lower, upper));
}

// ----------------------------------------------------------------------------
void QueryASTNode::destroySingle(QueryASTNode* node)
{
    delete node;
}

// ----------------------------------------------------------------------------
void QueryASTNode::destroyRecurse(QueryASTNode* node)
{
    switch (node->type)
    {
    case STATEMENT:
        destroyRecurse(node->statement.child);
        destroyRecurse(node->statement.next);
        break;
    case REPITITION:
        destroyRecurse(node->repitition.child);
        break;
    case UNION:
        destroyRecurse(node->union_.child);
        destroyRecurse(node->union_.next);
        break;
    case INVERSION:
        destroyRecurse(node->inversion.child);
        break;
    case WILDCARD:
        break;
    case LABEL:
        break;
    case CONTEXT_QUALIFIER:
        destroyRecurse(node->contextQualifier.child);
        break;
    case DAMAGE_RANGE:
        destroyRecurse(node->damageRange.child);
        break;
    }

    destroySingle(node);
}

// ----------------------------------------------------------------------------
static void calculateNodeIDs(const QueryASTNode* node, rfcommon::HashMap<const QueryASTNode*, int>* nodeIDs, int* counter)
{
    *counter += 1;
    nodeIDs->insertIfNew(node, *counter);

    switch (node->type)
    {
    case QueryASTNode::STATEMENT:
        calculateNodeIDs(node->statement.child, nodeIDs, counter);
        calculateNodeIDs(node->statement.next, nodeIDs, counter);
        break;
    case QueryASTNode::REPITITION:
        calculateNodeIDs(node->repitition.child, nodeIDs, counter);
        break;
    case QueryASTNode::UNION:
        calculateNodeIDs(node->union_.child, nodeIDs, counter);
        calculateNodeIDs(node->union_.next, nodeIDs, counter);
        break;
    case QueryASTNode::INVERSION:
        calculateNodeIDs(node->inversion.child, nodeIDs, counter);
        break;
    case QueryASTNode::WILDCARD:
        break;
    case QueryASTNode::LABEL:
        break;
    case QueryASTNode::CONTEXT_QUALIFIER:
        calculateNodeIDs(node->contextQualifier.child, nodeIDs, counter);
        break;
    case QueryASTNode::DAMAGE_RANGE:
        calculateNodeIDs(node->damageRange.child, nodeIDs, counter);
        break;
    }
}

static void writeNodes(const QueryASTNode* node, FILE* fp, const rfcommon::HashMap<const QueryASTNode*, int>& nodeIDs)
{
    const int nodeID = nodeIDs.find(node)->value();
    switch (node->type)
    {
    case QueryASTNode::STATEMENT:
        fprintf(fp, "  n%d [label=\"->\"];\n", nodeID);
        writeNodes(node->statement.child, fp, nodeIDs);
        writeNodes(node->statement.next, fp, nodeIDs);
        break;
    case QueryASTNode::REPITITION:
        fprintf(fp, "  n%d [label=\"rep %d,%d\"];\n",
                nodeID, node->repitition.minreps, node->repitition.maxreps);
        writeNodes(node->repitition.child, fp, nodeIDs);
        break;
    case QueryASTNode::UNION:
        fprintf(fp, "  n%d [label=\"|\"];\n", nodeID);
        writeNodes(node->union_.child, fp, nodeIDs);
        writeNodes(node->union_.next, fp, nodeIDs);
        break;
    case QueryASTNode::INVERSION:
        fprintf(fp, "  n%d [label=\"!\"];\n", nodeID);
        writeNodes(node->inversion.child, fp, nodeIDs);
        break;
    case QueryASTNode::WILDCARD:
        fprintf(fp, "  n%d [shape=\"rectangle\",label=\".\"];\n", nodeID);
        break;
    case QueryASTNode::LABEL:
        fprintf(fp, "  n%d [shape=\"rectangle\",label=\"%s\"];\n",
                nodeID, node->label.cStr());
        break;
    case QueryASTNode::CONTEXT_QUALIFIER: {
        rfcommon::SmallVector<rfcommon::SmallString<5>, 5> flags;
        if (!!(node->contextQualifier.flags & QueryASTNode::OS))
            flags.emplace("OS");
        if (!!(node->contextQualifier.flags & QueryASTNode::HIT))
            flags.emplace("HIT");
        if (!!(node->contextQualifier.flags & QueryASTNode::WHIFF))
            flags.emplace("WHIFF");

        fprintf(fp, "  n%d [shape=\"record\",label=\"", nodeID);
        for (int i = 0; i != flags.count(); ++i)
        {
            if (i == 0)
                fprintf(fp, "%s", flags[i].cStr());
            else
                fprintf(fp, " | %s", flags[i].cStr());
        }
        fprintf(fp, "\"];\n");
        writeNodes(node->contextQualifier.child, fp, nodeIDs);
    } break;
    case QueryASTNode::DAMAGE_RANGE:
        fprintf(fp, "  n%d [shape=\"record\",label=\"%d%%-%d%%\"];\n",
                nodeID, node->damageRange.lower, node->damageRange.upper);
        writeNodes(node->contextQualifier.child, fp, nodeIDs);
        break;
    }
}

static void writeEdges(const QueryASTNode* node, FILE* fp, const rfcommon::HashMap<const QueryASTNode*, int>& nodeIDs)
{
    switch (node->type)
    {
    case QueryASTNode::STATEMENT:
        fprintf(fp, "  n%d -> n%d;\n",
            nodeIDs.find(node)->value(), nodeIDs.find(node->statement.child)->value());
        fprintf(fp, "  n%d -> n%d;\n",
            nodeIDs.find(node)->value(), nodeIDs.find(node->statement.next)->value());
        writeEdges(node->statement.child, fp, nodeIDs);
        writeEdges(node->statement.next, fp, nodeIDs);
        break;
    case QueryASTNode::REPITITION:
        fprintf(fp, "  n%d -> n%d;\n",
            nodeIDs.find(node)->value(), nodeIDs.find(node->repitition.child)->value());
        writeEdges(node->repitition.child, fp, nodeIDs);
        break;
    case QueryASTNode::UNION:
        fprintf(fp, "  n%d -> n%d;\n",
            nodeIDs.find(node)->value(), nodeIDs.find(node->union_.child)->value());
        fprintf(fp, "  n%d -> n%d;\n",
            nodeIDs.find(node)->value(), nodeIDs.find(node->union_.next)->value());
        writeEdges(node->union_.child, fp, nodeIDs);
        writeEdges(node->union_.next, fp, nodeIDs);
        break;
    case QueryASTNode::INVERSION:
        fprintf(fp, "  n%d -> n%d;\n",
            nodeIDs.find(node)->value(), nodeIDs.find(node->inversion.child)->value());
        writeEdges(node->inversion.child, fp, nodeIDs);
        break;
    case QueryASTNode::WILDCARD:
        break;
    case QueryASTNode::LABEL:
        break;
    case QueryASTNode::CONTEXT_QUALIFIER:
        fprintf(fp, "  n%d -> n%d;\n",
            nodeIDs.find(node)->value(), nodeIDs.find(node->contextQualifier.child)->value());
        writeEdges(node->contextQualifier.child, fp, nodeIDs);
        break;
    case QueryASTNode::DAMAGE_RANGE:
        fprintf(fp, "  n%d -> n%d;\n",
            nodeIDs.find(node)->value(), nodeIDs.find(node->damageRange.child)->value());
        writeEdges(node->damageRange.child, fp, nodeIDs);
        break;
    }
}

void QueryASTNode::exportDOT(const char* filename) const
{
    int counter = 0;
    rfcommon::HashMap<const QueryASTNode*, int> nodeIDs;
    calculateNodeIDs(this, &nodeIDs, &counter);

    FILE* fp = fopen(filename, "w");
    fprintf(fp, "digraph ast {\n");
        writeNodes(this, fp, nodeIDs);
        writeEdges(this, fp, nodeIDs);
    fprintf(fp, "}\n");
    fclose(fp);
}
