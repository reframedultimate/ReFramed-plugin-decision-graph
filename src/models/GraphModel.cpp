#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/Vector.hpp"

#include "ogdf/basic/Graph.h"
#include "ogdf/basic/GraphAttributes.h"
#include "ogdf/planarity/PlanarizationLayout.h"
#include "ogdf/energybased/FMMMLayout.h"
#include "ogdf/layered/FastHierarchyLayout.h"
#include "ogdf/layered/SugiyamaLayout.h"
#include "ogdf/layered/MedianHeuristic.h"
#include "ogdf/layered/OptimalHierarchyLayout.h"
#include "ogdf/layered/OptimalRanking.h"

#include <QGraphicsSimpleTextItem>

// ----------------------------------------------------------------------------
GraphModel::GraphModel(SequenceSearchModel* searchModel, rfcommon::MotionLabels* labels)
    : searchModel_(searchModel)
    , labels_(labels)
    , preferredLayer_(labels->preferredLayer(rfcommon::MotionLabels::NOTATION))
{
    searchModel_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
GraphModel::~GraphModel()
{
    searchModel_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void GraphModel::setOutgoingTreeSize(int size)
{
    outgoingTree_ = size;
    redrawGraph();
}

// ----------------------------------------------------------------------------
void GraphModel::setIncomingTreeSize(int size)
{
    incomingTree_ = size;
    redrawGraph();
}

// ----------------------------------------------------------------------------
void GraphModel::setMergeBehavior(MergeBehavior behavior)
{
    mergeBehavior_ = behavior;
    redrawGraph();
}

// ----------------------------------------------------------------------------
void GraphModel::setPreferredLayer(int layerIdx)
{
    preferredLayer_ = layerIdx;
    redrawGraph();
}

// ----------------------------------------------------------------------------
void GraphModel::setUseLargestIsland(bool enable)
{
    useLargestIsland_ = enable;
    redrawGraph();
}

// ----------------------------------------------------------------------------
void GraphModel::setFixEdges(bool enable)
{
    fixEdges_ = enable;
    redrawGraph();
}

// ----------------------------------------------------------------------------
void GraphModel::setMergeQualifiers(bool enable)
{
    mergeQualifiers_ = enable;
    redrawGraph();
}

// ----------------------------------------------------------------------------
void GraphModel::setShowHash40Values(bool enable)
{
    showHash40Values_ = enable;
    redrawGraph();
}

// ----------------------------------------------------------------------------
void GraphModel::setShowQualifiers(bool enable)
{
    showQualifiers_ = enable;
    redrawGraph();
}

// ---------------------------------------------------------------------------
int GraphModel::availableLayersCount() const
{
    return labels_->layerCount();
}

// ----------------------------------------------------------------------------
rfcommon::String GraphModel::availableLayerName(int idx) const
{
    return rfcommon::String(labels_->layerGroup(idx)) + " - " + labels_->layerName(idx);
}

// ----------------------------------------------------------------------------
void GraphModel::redrawGraph()
{
    clear();

    if (searchModel_->playerPOV() < 0)
        return;

    const States& states = searchModel_->fighterStates(searchModel_->playerPOV());
    rfcommon::FighterID fighterID = searchModel_->fighterID(searchModel_->playerPOV());

    auto motionString = [this](rfcommon::FighterID fighterID, const State& state) -> rfcommon::String {
        if (const char* notation = labels_->toGroupLabel(fighterID, state.motion, preferredLayer_))
            return notation;
        else if (const char* h40 = labels_->toHash40(state.motion))
            return h40;
        else
            return state.motion.toHex();
    };

    auto hash40String = [this](const State& state) -> rfcommon::String {
        if (const char* h40 = labels_->toHash40(state.motion))
            return h40;
        return state.motion.toHex();
    };

    auto qualifiersString = [this](const State& state) -> rfcommon::String {
        rfcommon::String flags;
        switch (state.interaction())
        {
            case State::NO_INTERACTION: break;
            case State::TRADE: flags += "trade"; break;
            case State::ADVANTAGE: flags += "adv"; break;
            case State::DISADVANTAGE: flags += "disadv"; break;
            case State::ON_SHIELD: flags += "os"; break;
            case State::SHIELD_LAG: flags += "shieldlag"; break;
        }

        return flags;
    };

    // Convert sequences to graph with the appropriate merge behavior
    Graph graph;
    switch (mergeBehavior_)
    {
        case DONT_MERGE:
            for (int queryIdx = 0; queryIdx != searchModel_->queryCount(); ++queryIdx)
                graph.addStates(states, searchModel_->matches(queryIdx));
            break;

        case QUERY_MERGE:
            for (int queryIdx = 0; queryIdx != searchModel_->queryCount(); ++queryIdx)
                graph.addStates(states, searchModel_->mergedMatches(queryIdx));
            break;

        case LABEL_MERGE:
            for (int queryIdx = 0; queryIdx != searchModel_->queryCount(); ++queryIdx)
            {
                rfcommon::Vector<Sequence> mergedSequences;
                rfcommon::HashMap<rfcommon::String, int> lookup;
                for (const Range& range : searchModel_->matches(queryIdx))
                {
                    Sequence& seq = mergedSequences.emplace();
                    for (int stateIdx = range.startIdx; stateIdx != range.endIdx; ++stateIdx)
                    {
                        int mergeIdx = lookup.insertOrGet(motionString(fighterID, states[stateIdx]), stateIdx)->value();
                        seq.idxs.push(mergeIdx);
                    }
                }
                graph.addStates(states, mergedSequences);
            }
            break;
    }

    if (graph.nodes.count() == 0)
        return;

    // The algorithms for calculating incoming and outgoing trees assume that
    // all nodes are connected. Need to create a list of islands and apply those
    // algorithms on each one separately. Additionally, if the user only wants
    // to see the the largest island, find that instead.
    rfcommon::Vector<Graph> islands = graph.islands();
    if (useLargestIsland_)
    {
        // Find largest island
        int largest = 0;
        int nodes = 0;
        for (int i = 0; i != islands.count(); ++i)
            if (nodes < islands[i].nodes.count())
            {
                nodes = islands[i].nodes.count();
                largest = i;
            }

        islands = rfcommon::Vector<Graph>({ islands[largest] });
    }

    graph = Graph();
    for (const Graph& island : islands)
        graph.addIsland(island);

    ogdf::Graph G;
    ogdf::GraphAttributes GA(G,
        ogdf::GraphAttributes::nodeGraphics
      | ogdf::GraphAttributes::edgeGraphics
      | ogdf::GraphAttributes::nodeLabel
      | ogdf::GraphAttributes::edgeStyle
      | ogdf::GraphAttributes::nodeStyle
      | ogdf::GraphAttributes::nodeTemplate);

    rfcommon::Vector<ogdf::node> ogdfNodes;
    rfcommon::Vector<ogdf::edge> ogdfEdges;
    rfcommon::Vector<QGraphicsSimpleTextItem*> labels;
    rfcommon::Vector<QGraphicsSimpleTextItem*> hash40Strings;
    rfcommon::Vector<QGraphicsSimpleTextItem*> hash40Values;

    for (const auto& node : graph.nodes)
    {
        if (showQualifiers_)
        {
            rfcommon::String label = motionString(fighterID, states[node.stateIdx]);
            rfcommon::String qual = qualifiersString(states[node.stateIdx]);
            if (qual.notEmpty())
                label += " " + qual;
            labels.push(new QGraphicsSimpleTextItem(QString::fromUtf8(
                 label.cStr()
            )));
        }
        else
        {
            labels.push(new QGraphicsSimpleTextItem(QString::fromUtf8(
                 motionString(fighterID, states[node.stateIdx]).cStr()
            )));
        }

        if (showHash40Values_)
        {
            hash40Strings.push(new QGraphicsSimpleTextItem(QString::fromUtf8(
                hash40String(states[node.stateIdx]).cStr()
            )));
            hash40Values.push(new QGraphicsSimpleTextItem(QString::fromUtf8(
                states[node.stateIdx].motion.toHex().cStr()
            )));
        }

        double w = labels.back()->boundingRect().width();
        double h = labels.back()->boundingRect().height();

        if (showHash40Values_)
        {
            if (w < hash40Strings.back()->boundingRect().width())
                w = hash40Strings.back()->boundingRect().width();
            if (w < hash40Values.back()->boundingRect().width())
                w = hash40Values.back()->boundingRect().width();
            h += hash40Strings.back()->boundingRect().height() + 2;
            h += hash40Values.back()->boundingRect().height() + 2;
        }

        ogdf::node N = G.newNode();
        GA.width(N) = w + 4;
        GA.height(N) = h + 4;
        ogdfNodes.push(N);
    }

    for (int edgeIdx = 0; edgeIdx != graph.edges.count(); ++edgeIdx)
    {
        const int from = graph.edges[edgeIdx].from;
        const int to = graph.edges[edgeIdx].to;
        ogdf::edge E = G.newEdge(ogdfNodes[from], ogdfNodes[to]);
        ogdfEdges.push(E);
    }

    ogdf::setSeed(42);
    ogdf::SugiyamaLayout layout;
    layout.setRanking(new ogdf::OptimalRanking);
    layout.setCrossMin(new ogdf::MedianHeuristic);

    ogdf::OptimalHierarchyLayout* ohl = new ogdf::OptimalHierarchyLayout;
    ohl->layerDistance(30.0);
    ohl->nodeDistance(25.0);
    ohl->weightBalancing(0.8);
    layout.setLayout(ohl);
    layout.call(GA);

    for (int nodeIdx = 0; nodeIdx != ogdfNodes.count(); ++nodeIdx)
    {
        double x = GA.x(ogdfNodes[nodeIdx]);
        double y = GA.y(ogdfNodes[nodeIdx]);
        double w = GA.width(ogdfNodes[nodeIdx]);
        double h = GA.height(ogdfNodes[nodeIdx]);

        QRectF rect(x - w/2, y - h/2, w, h);
        QGraphicsRectItem* nodeShape = addRect(rect, QPen(QColor("green")));

        QGraphicsSimpleTextItem* label = labels[nodeIdx];
        rect = label->boundingRect();
        double cx = (w - rect.width()) / 2;
        double cy = (h - rect.height()) / 2;
        label->setParentItem(nodeShape);

        if (showHash40Values_)
            label->setPos(x - w/2 + cx, y - h/4*3 + cy);
        else
            label->setPos(x - w/2 + cx, y - h/2 + cy);

        if (showHash40Values_)
        {
            label = hash40Strings[nodeIdx];
            rect = label->boundingRect();
            cx = (w - rect.width()) / 2;
            cy = (h - rect.height()) / 2;
            label->setParentItem(nodeShape);
            label->setPos(x - w/2 + cx, y - h/4*2 + cy);

            label = hash40Values[nodeIdx];
            rect = label->boundingRect();
            cx = (w - rect.width()) / 2;
            cy = (h - rect.height()) / 2;
            label->setParentItem(nodeShape);
            label->setPos(x - w/2 + cx, y - h/4*1 + cy);
        }
    }

    // When drawing edges, we need the lines to start and end on the boundary
    // of the rectangle shape rather than in the center. Takes the center and
    // width/height of a rectangle, and projects the point "p" onto the outline.
    auto intersectRect = [](double cx, double cy, double w, double h, double px, double py) -> ogdf::DPoint {
        double dx = cx - px;
        double dy = cy - py;

        double off;
        if (dy != 0.0)
            off = dx / dy * h/2;
        if (dy != 0.0 && off >= -w/2 && off <= w/2)
        {
            if (py > cy)
                return ogdf::DPoint(cx + off, cy + h/2);
            else
                return ogdf::DPoint(cx - off, cy - h/2);
        }
        else
        {
            if (px > cx)
                return ogdf::DPoint(cx + w/2, cy + dy / dx * w/2);
            else
                return ogdf::DPoint(cx - w/2, cy - dy / dx * w/2);
        }
    };

    for (int edgeIdx = 0; edgeIdx != ogdfEdges.count(); ++edgeIdx)
    {
        ogdf::edge edge = ogdfEdges[edgeIdx];

        // --------------------------------------------------------------------
        // Draw path
        // --------------------------------------------------------------------

        ogdf::DPoint start = GA.point(edge->source());
        const ogdf::DPolyline& bends = GA.bends(edge);
        ogdf::DPoint end = GA.point(edge->target());

        // Move start and end points to be on the surface of the node's shape,
        // rather than the center
        start = intersectRect(
            start.m_x, start.m_y,
            GA.width(edge->source()), GA.height(edge->source()),
            bends.empty() ? end.m_x : bends.front().m_x,
            bends.empty() ? end.m_y : bends.front().m_y
        );
        end = intersectRect(
            end.m_x, end.m_y,
            GA.width(edge->target()), GA.height(edge->target()),
            bends.empty() ? start.m_x : bends.back().m_x,
            bends.empty() ? start.m_y : bends.back().m_y
        );

        QPainterPath path(QPointF(start.m_x, start.m_y));
        for (ogdf::ListConstIterator<ogdf::DPoint> it = bends.begin(); it != bends.end(); ++it)
            path.lineTo(QPointF((*it).m_x, (*it).m_y));
        path.lineTo(QPointF(end.m_x, end.m_y));

        QGraphicsPathItem* pathItem = addPath(path, QPen(QColor("green")));

        // --------------------------------------------------------------------
        // Draw arrow head
        // --------------------------------------------------------------------

        ogdf::DPoint arrowStartPoint = bends.empty() ? start : bends.back();
        ogdf::DPoint arrowEndPoint = end;

        qreal Pi = 3.14159;
        qreal arrowSize = 10;

        QLineF line(
            QPointF(arrowEndPoint.m_x, arrowEndPoint.m_y),
            QPointF(arrowStartPoint.m_x, arrowStartPoint.m_y)
        );

        double angle = ::acos(line.dx() / line.length());

        if (line.dy() >= 0)
            angle = (Pi * 2) - angle;

        QPointF arrowP1 = line.p1() + QPointF(sin(angle + Pi / 3) * arrowSize,
            cos(angle + Pi / 3) * arrowSize);
        QPointF arrowP2 = line.p1() + QPointF(sin(angle + Pi - Pi / 3) * arrowSize,
            cos(angle + Pi - Pi / 3) * arrowSize);

        QPolygonF arrowHead;
        arrowHead << line.p1() << arrowP1 << arrowP2;

        addPolygon(arrowHead, QPen(QColor("green")), QBrush(QColor("green")));

        // --------------------------------------------------------------------
        // Draw edge label
        // --------------------------------------------------------------------

        //QGraphicsSimpleTextItem* edgeLabel = new QGraphicsSimpleTextItem(QString::number(graph.edges[edgeIdx].weight()));
        //edgeLabel->setParentItem(pathItem);
        QGraphicsSimpleTextItem* edgeLabel = addSimpleText(QString::number(graph.edges[edgeIdx].weight));
        QPointF edgeLabelPos(
            end.m_x - edgeLabel->boundingRect().width()/2,
            end.m_y - edgeLabel->boundingRect().height()/2
        );
        edgeLabelPos -= QPointF(-std::cos(angle), std::sin(angle)) * edgeLabel->boundingRect().height();
        edgeLabel->setPos(edgeLabelPos);
    }

    //ogdf::GraphIO::write(GA, "graph.svg", ogdf::GraphIO::drawSVG);
}

// ----------------------------------------------------------------------------
void GraphModel::onNewSessions() {}
void GraphModel::onClearAll()
{
    clear();
}
void GraphModel::onDataAdded() {}
void GraphModel::onPOVChanged() {}
void GraphModel::onQueriesChanged() {}
void GraphModel::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void GraphModel::onQueriesApplied()
{
    redrawGraph();
}
