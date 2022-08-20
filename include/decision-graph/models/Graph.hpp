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

    struct UniqueSequence
    {
        Sequence sequence;
        int weight;
    };

    static Graph fromSequenceRanges(const Sequence& sequence, const rfcommon::Vector<SequenceRange>& ranges);

    int findHighestThroughputNode() const;

    /*!
     * \brief Returns a list of all unconnected sub-graphs. If the graph is
     * fully connected then this will return a list of 1 graph.
     */
    rfcommon::Vector<Graph> islands() const;

    /*!
     * \brief Tries to eliminate all edge connections that loop back into the 
     * graph. The result is a graph with no cycles and mostly sink leaf nodes.
     * The first node in the graph is the root node of the tree.
     * \note Assumes every node in the graph is connected
     */
    Graph outgoingTree() const;

    /*!
     * \brief Tries to eliminate all edge connections that loop out of the
     * graph. The result is a graph with no cycles and mostly source leaf nodes.
     * The first node in the graph is the root node of the tree.
     * \note Assumes every node in the graph is connected
     */
    Graph incomingTree() const;

    rfcommon::Vector<UniqueSequence> treeToUniuqeOutgoingSequences() const;

    rfcommon::Vector<UniqueSequence> treeToUniqueIncomingSequences() const;

    void exportDOT(const char* fileName, rfcommon::FighterID fighterID, const LabelMapper* labels) const;
    void exportOGDFSVG(const char* fileName, rfcommon::FighterID fighterID, const LabelMapper* labels) const;
};
