#pragma once

#include "decision-graph/listeners/GraphModelListener.hpp"
#include "decision-graph/widgets/PropertyWidget.hpp"
#include "rfcommon/MotionLabelsListener.hpp"

class GraphModel;
class QComboBox;

class PropertyWidget_Graph
        : public PropertyWidget
        , public GraphModelListener
        , public rfcommon::MotionLabelsListener
{
public:
    PropertyWidget_Graph(GraphModel* graphModel, SequenceSearchModel* searchModel, rfcommon::MotionLabels* labels, QWidget* parent=nullptr);
    ~PropertyWidget_Graph();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:
    void updateAvailableLayersDropdown();

private:
    void onGraphModelPreferredLayerChanged() override;

private:
    void onMotionLabelsLoaded() override;
    void onMotionLabelsHash40sUpdated() override;

    void onMotionLabelsPreferredLayerChanged(int usage) override;

    void onMotionLabelsLayerInserted(int layerIdx) override;
    void onMotionLabelsLayerRemoved(int layerIdx) override;
    void onMotionLabelsLayerNameChanged(int layerIdx) override;
    void onMotionLabelsLayerUsageChanged(int layerIdx, int oldUsage) override;
    void onMotionLabelsLayerMoved(int fromIdx, int toIdx) override;
    void onMotionLabelsLayerMerged(int layerIdx) override;

    void onMotionLabelsRowInserted(rfcommon::FighterID fighterID, int row) override;
    void onMotionLabelsLabelChanged(rfcommon::FighterID fighterID, int row, int layerIdx) override;
    void onMotionLabelsCategoryChanged(rfcommon::FighterID fighterID, int row, int oldCategory) override;

private:
    GraphModel* graphModel_;
    rfcommon::MotionLabels* labels_;
    QComboBox* comboBox_layer;
};
