#pragma once

#include "rfcommon/HashMap.hpp"

class Edge
{
public:
    Edge(int from, int to)
        : from_(from), to_(to), weight_(1)
    {}
    Edge(int from, int to, int weight)
        : from_(from), to_(to), weight_(weight)
    {}

    int from() const { return from_; }
    int to() const { return to_; }

    void addWeight() { weight_++; }
    int weight() const { return weight_; }

private:
    int from_, to_;
    int weight_;
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
        typedef rfcommon::HashMapHasher<EdgeConnection>::HashType HashType;
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
