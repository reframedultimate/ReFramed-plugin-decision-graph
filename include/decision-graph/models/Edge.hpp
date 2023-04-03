#pragma once

#include "rfcommon/HashMap.hpp"

class Edge
{
public:
    Edge(int from, int to)
        : from(from), to(to), weight(1)
    {}
    Edge(int from, int to, int weight)
        : from(from), to(to), weight(weight)
    {}

    int from, to;
    int weight;
};

// When looking for existing connections in the graph, we do
// not care about the weight or any other edge attribute
class EdgeConnection
{
public:
    EdgeConnection(int from, int to)
        : from_(from), to_(to)
    {}

    struct Hasher {
        typedef uint32_t HashType;
        HashType operator()(const EdgeConnection& edge) const {
            const uint32_t data[2] = {
                static_cast<uint32_t>(edge.from_),
                static_cast<uint32_t>(edge.to_)
            };

            return rfcommon::hash32_jenkins_oaat(&data, 8);
        }
    };

    bool operator==(const EdgeConnection& other) const
    {
        return from_ == other.from_ &&
                to_ == other.to_;
    }

private:
    int from_, to_;
};
