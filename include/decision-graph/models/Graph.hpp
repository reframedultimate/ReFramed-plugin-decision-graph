#pragma once

#include "decision-graph/models/Edge.hpp"
#include "decision-graph/models/Node.hpp"
#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"

class LabelMapper;

class Graph
{
public:
    rfcommon::Vector<Node> nodes;
    rfcommon::Vector<Edge> edges;

    static Graph fromSequenceRanges(const Sequence& sequence, const rfcommon::Vector<SequenceRange>& ranges);

    struct UniqueSequence
    {
        Sequence sequence;
        int weight;
    };

    /*!
     * \brief Tries to eliminate all edge connections that loop back into the 
     * graph. The result is a graph with no cycles and mostly sink leaf nodes.
     */
    Graph cutLoopsIncoming() const;

    /*!
     * \brief Tries to eliminate all edge connections that loop out of the
     * graph. The result is a graph with no cycles and mostly source leaf nodes.
     */
    Graph cutLoopsOutgoing() const;

    /*!
     * \brief Returns a list of all unconnected sub-graphs. If the graph is
     * fully connected then this will return a list of 1 graph.
     */
    rfcommon::Vector<Graph> islands() const;

    /*!
     * \brief Returns a list of all unconnected sub-trees. If the tree is
     * fully connected then this will return a list of 1 tree.
     * \note Assumes the graph has no cycles.
     */
    rfcommon::Vector<Graph> treeIslands() const;

    /*!
     * \brief Finds all sink leaf nodes and creates sequences that all share
     * a common ancestor.
     * \note Assumes the graph has no cycles.
     */
    rfcommon::Vector<UniqueSequence> uniqueSinks() const;

    /*!
     * \brief Finds all source leaf nodes and creates sequences that all share
     * a common ancestor.
     * \note Assumes the graph has no cycles.
     */
    rfcommon::Vector<UniqueSequence> uniqueSources() const;

    void exportDOT(const char* fileName, rfcommon::FighterID fighterID, const LabelMapper* labels) const;
    void exportOGDFSVG(const char* fileName, rfcommon::FighterID fighterID, const LabelMapper* labels) const;
};
