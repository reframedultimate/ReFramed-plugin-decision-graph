#pragma once

#include "decision-graph/models/Edge.hpp"
#include "decision-graph/models/Node.hpp"
#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"

class LabelMapper;

class Graph
{
    Graph(const States& states);
public:
    ~Graph();

    struct UniqueSequence
    {
        UniqueSequence(Sequence&& seq, int weight)
            : sequence(std::move(seq))
            , weight(weight)
        {}

        Sequence sequence;
        int weight;
    };

    static Graph fromSequences(const States& states, const rfcommon::Vector<Sequence>& sequences);

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

    void exportDOT(const char* fileName, const LabelMapper* labels) const;
    void exportOGDFSVG(const char* fileName, const LabelMapper* labels) const;

    rfcommon::Vector<Node> nodes;
    rfcommon::Vector<Edge> edges;
    const States& states;
};
