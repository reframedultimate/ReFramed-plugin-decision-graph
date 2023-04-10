#include "decision-graph/models/RegionItem.hpp"

// ----------------------------------------------------------------------------
RegionItem::RegionItem()
{
    setAcceptHoverEvents(true);
    setFlags(flags()
        | QGraphicsItem::ItemIsMovable
        | QGraphicsItem::ItemIsSelectable
        | QGraphicsItem::ItemSendsGeometryChanges
        | QGraphicsItem::ItemIsFocusable);
}

// ----------------------------------------------------------------------------
RegionItem::~RegionItem()
{}
