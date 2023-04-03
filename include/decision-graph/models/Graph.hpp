#pragma once

#include "decision-graph/models/Edge.hpp"
#include "decision-graph/models/Node.hpp"
#include "decision-graph/models/Sequence.hpp"

#include "rfcommon/Vector.hpp"

class LabelMapper;

class Graph
{
public:
    Graph();
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

    void clear();

    Graph& addStates(const States& states, const rfcommon::Vector<Range>& ranges);
    Graph& addStates(const States& states, const rfcommon::Vector<Sequence>& sequences);

    Graph& addIsland(const Graph& other);

    int findHighestThroughputNode() const;
    int findHighestThroughputEdge() const;

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
    Graph outgoingTree(const States& states) const;

    /*!
     * \brief Tries to eliminate all edge connections that loop out of the
     * graph. The result is a graph with no cycles and mostly source leaf nodes.
     * The first node in the graph is the root node of the tree.
     * \note Assumes every node in the graph is connected
     */
    Graph incomingTree(const States& states) const;

    rfcommon::Vector<UniqueSequence> treeToUniuqeOutgoingSequences() const;

    rfcommon::Vector<UniqueSequence> treeToUniqueIncomingSequences() const;

    void exportDOT(const char* fileName, const States& states, const rfcommon::MotionLabels* labels) const;

    rfcommon::Vector<Node> nodes;
    rfcommon::Vector<Edge> edges;
};
