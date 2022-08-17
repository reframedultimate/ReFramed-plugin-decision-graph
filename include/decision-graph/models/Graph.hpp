#pragma once

#include "decision-graph/models/Edge.hpp"
#include "decision-graph/models/Node.hpp"
#include "decision-graph/models/Sequence.hpp"
#include "rfcommon/Vector.hpp"
#include "rfcommon/FighterID.hpp"

class LabelMapper;

namespace rfcommon {
    class MappingInfo;
};

class Graph
{
public:
    rfcommon::Vector<Node> nodes;
    rfcommon::Vector<Edge> edges;

public:
    static Graph fromSequenceRanges(const Sequence& sequence, const rfcommon::Vector<SequenceRange>& ranges);

    void exportDOT(const char* fileName, rfcommon::FighterID fighterID, const rfcommon::MappingInfo* map, const LabelMapper* labels) const;
    void exportOGDFSVG(const char* fileName, rfcommon::FighterID fighterID, const rfcommon::MappingInfo* map, const LabelMapper* labels) const;
};
