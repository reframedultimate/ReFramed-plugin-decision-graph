#pragma once

#include "decision-graph/models/DecisionGraph.hpp"

class GraphQuery
{
public:
    static GraphQuery parse(const char* string);

    DecisionGraph filter(const DecisionGraph& graph);
};
