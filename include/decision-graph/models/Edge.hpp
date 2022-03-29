#pragma once

class Edge
{
public:
    Edge(int from, int to)
        : from_(from), to_(to), weight_(1)
    {}
    Edge(int from, int to, int weight)
        : from_(from), to_(to), weight_(weight)
    {}

    bool operator==(const Edge& other) const
    {
        return from_ == other.from_ &&
                to_ == other.to_ &&
                weight_ == other.weight_;
    }

    int from() const { return from_; }
    int to() const { return to_; }

    void addWeight() { weight_++; }
    int weight() const { return weight_; }

private:
    int from_, to_;
    int weight_;
};
