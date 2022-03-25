#pragma once

#include "decision-graph/models/DecisionGraph.hpp"
#include "rfcommon/Vector.hpp"

class GraphQuery
{
public:
    DecisionGraph apply(const DecisionGraph& graph);

private:
};
