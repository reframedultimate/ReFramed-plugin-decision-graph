#include "decision-graph/views/GraphView.hpp"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

#include "ogdf/basic/Graph.h"
#include "ogdf/basic/GraphAttributes.h"
#include "ogdf/planarity/PlanarizationLayout.h"
#include "ogdf/energybased/FMMMLayout.h"
#include "ogdf/layered/FastHierarchyLayout.h"
#include "ogdf/layered/SugiyamaLayout.h"

#include "ogdf/fileformats/GraphIO.h"

#include "rfcommon/MotionLabels.hpp"
#include "rfcommon/Vector.hpp"

#include <QGraphicsSimpleTextItem>

// ----------------------------------------------------------------------------
GraphView::GraphView(GraphModel* graphModel, SequenceSearchModel* searchModel, rfcommon::MotionLabels* labels, QWidget* parent)
    : QGraphicsView(parent)
    , graphModel_(graphModel)
    , searchModel_(searchModel)
    , labels_(labels)
{
    setScene(graphModel);
    setDragMode(ScrollHandDrag);
    setInteractive(false);

    searchModel_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
GraphView::~GraphView()
{
    searchModel_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void GraphView::onNewSessions() {}
void GraphView::onClearAll()
{
    graphModel_->clear();
}
void GraphView::onDataAdded() {}
void GraphView::onPOVChanged() {}
void GraphView::onQueriesChanged() {}
void GraphView::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) {}
void GraphView::onQueriesApplied()
{
    const States& states = searchModel_->fighterStates(searchModel_->playerPOV());
    rfcommon::FighterID fighterID = searchModel_->fighterID(searchModel_->playerPOV());

    Graph fullGraph;
    for (int queryIdx = 0; queryIdx != searchModel_->queryCount(); ++queryIdx)
        fullGraph.addSequences(states, searchModel_->mergedMatches(queryIdx));

    // Find largest island
    const auto islands = fullGraph.islands();
    if (islands.count() == 0)
        return;

    int largest = 0;
    int nodes = 0;
    for (int i = 0; i != islands.count(); ++i)
        if (nodes < islands[i].nodes.count())
        {
            nodes = islands[i].nodes.count();
            largest = i;
        }

    const Graph& graph = islands[largest];

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
    rfcommon::Vector<QGraphicsSimpleTextItem*> nodeLabels;

    auto motionToString = [this](rfcommon::FighterID fighterID, rfcommon::FighterMotion motion) -> rfcommon::String {
        if (const char* label = labels_->toPreferredNotation(fighterID, motion))
            return label;
        if (const char* h40 = labels_->lookupHash40(motion))
            return h40;
        return motion.toHex();
    };

    for (const auto& node : graph.nodes)
    {
        nodeLabels.push(new QGraphicsSimpleTextItem(QString::fromUtf8(
            motionToString(fighterID, states[node.stateIdx].motion).cStr()
        )));

        ogdf::node N = G.newNode();
        GA.width(N) = nodeLabels.back()->boundingRect().width() + 4;
        GA.height(N) = nodeLabels.back()->boundingRect().height() + 4;
        ogdfNodes.push(N);
    }

    for (int edgeIdx = 0; edgeIdx != graph.edges.count(); ++edgeIdx)
    {
        const int from = graph.edges[edgeIdx].from();
        const int to = graph.edges[edgeIdx].to();
        ogdf::edge E = G.newEdge(ogdfNodes[from], ogdfNodes[to]);
        ogdfEdges.push(E);
    }

#if 0
    ogdf::FMMMLayout fmmm;
    fmmm.useHighLevelOptions(true);
    fmmm.unitEdgeLength(15.0);
    fmmm.newInitialPlacement(true);
    fmmm.qualityVersusSpeed(ogdf::FMMMOptions::QualityVsSpeed::GorgeousAndEfficient);
    fmmm.call(GA);
#elif 0
    ogdf::PlanarizationLayout layout;
    layout.call(GA);
#elif 0
    ogdf::FastHierarchyLayout layout;
    layout.call(GA);
#elif 1
    ogdf::SugiyamaLayout layout;
    layout.call(GA);
#endif

    graphModel_->clear();

    for (int nodeIdx = 0; nodeIdx != ogdfNodes.count(); ++nodeIdx)
    {
        double x = GA.x(ogdfNodes[nodeIdx]);
        double y = GA.y(ogdfNodes[nodeIdx]);
        double w = GA.width(ogdfNodes[nodeIdx]);
        double h = GA.height(ogdfNodes[nodeIdx]);

        QRectF rect(x - w / 2, y - h / 2, w, h);
        QGraphicsRectItem* ri = graphModel_->addRect(rect, QPen(QColor("green")));

        QGraphicsSimpleTextItem* label = nodeLabels[nodeIdx];
        QRectF labelRect = label->boundingRect();
        double newx = (w - labelRect.width()) / 2;
        double newy = (h - labelRect.height()) / 2;
        label->setBrush(QColor("black"));
        label->setParentItem(ri);
        label->setPos(x - w / 2 + newx, y - h / 2 + newy);
    }

    for (int edgeIdx = 0; edgeIdx != ogdfEdges.count(); ++edgeIdx)
    {
        ogdf::edge edge = ogdfEdges[edgeIdx];
        ogdf::DPolyline& bends = GA.bends(edge);

        ogdf::DPoint start = GA.point(edge->source());
        QPainterPath path(QPointF(start.m_x, start.m_y));

        for (ogdf::ListConstIterator<ogdf::DPoint> it = path.begin(); it.succ().valid(); ++it)

        ogdf::List<ogdf::DPoint>::const_iterator it = points.begin();
        if (it != points.end())
        {
            QPointF startPoint((*it).m_x, (*it).m_y);

            for (; it != points.end(); ++it)
            {
                ogdf::DPoint dp = *it;
                QPointF nextPoint(dp.m_x, dp.m_y);
                path.lineTo(nextPoint);
            }

            graphModel_->addPath(path, QPen(QColor("green")));
            /*
            ogdf::List<ogdf::DPoint>::iterator arrowStartPoint = points.get(points.size() - 2);
            ogdf::List<ogdf::DPoint>::iterator arrowEndPoint = points.get(points.size() - 1);

            qreal Pi = 3.14;
            qreal arrowSize = 10;

            QLineF line(
                QPointF((*arrowEndPoint).m_x, (*arrowEndPoint).m_y),
                QPointF((*arrowStartPoint).m_x, (*arrowStartPoint).m_y)
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

            graphModel_->addPolygon(arrowHead, QPen(QColor("green")), QBrush(QColor("green")));
            graphModel_->addLine(line, QPen(QColor("green")));
            */
        }
    }

    ogdf::GraphIO::write(GA, "graph.svg", ogdf::GraphIO::drawSVG);
}
