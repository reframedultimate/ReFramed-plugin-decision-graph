#pragma once

#include "decision-graph/models/Edge.hpp"
#include "decision-graph/models/Node.hpp"
#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"

namespace rfcommon {
    class Session;
};

class UserLabelsModel;

class Graph
{
public:
    rfcommon::Vector<Node> nodes;
    rfcommon::Vector<Edge> edges;

public:
    static Graph fromSequenceRanges(const Sequence& sequence, const rfcommon::Vector<SequenceRange>& ranges);

    void exportDOT(const char* fileName, int fighterIdx, const rfcommon::Session* session, const UserLabelsModel* table) const;
    void exportOGDFSVG(const char* fileName, int fighterIdx, const rfcommon::Session* session, const UserLabelsModel* table) const;
};
