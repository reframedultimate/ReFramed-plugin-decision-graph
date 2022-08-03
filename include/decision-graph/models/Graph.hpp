#pragma once

#include "decision-graph/models/Edge.hpp"
#include "decision-graph/models/Node.hpp"
#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"

namespace rfcommon {
    class MappingInfo;
};

class UserLabelsModel;

class Graph
{
public:
    rfcommon::Vector<Node> nodes;
    rfcommon::Vector<Edge> edges;

public:
    static Graph fromSequenceRanges(const Sequence& sequence, const rfcommon::Vector<SequenceRange>& ranges);

    void exportDOT(const char* fileName, rfcommon::FighterID fighterID, const rfcommon::MappingInfo* map, const UserLabelsModel* table) const;
    void exportOGDFSVG(const char* fileName, rfcommon::FighterID fighterID, const rfcommon::MappingInfo* map, const UserLabelsModel* table) const;
};
