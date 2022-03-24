#pragma once

#include "rfcommon/Vector.hpp"
#include "rfcommon/HashMap.hpp"

class DecisionGraph
{
public:
    struct Edge
    {
        int from, to;
        uint32_t frame;
        float damage;
    };

public:
    void addNode(uint16_t state);
    void addEdge(uint16_t from, uint16_t to, uint32_t frame, float damage);

private:
    rfcommon::HashMap<uint16_t, char> states_;
    rfcommon::Vector<Edge> edges_;
};
