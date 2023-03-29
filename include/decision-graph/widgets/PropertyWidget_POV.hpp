#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "decision-graph/widgets/PropertyWidget.hpp"

class QComboBox;

class PropertyWidget_POV
        : public PropertyWidget
        , public SequenceSearchListener
{
public:
    PropertyWidget_POV(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_POV();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private:
    void onComboBoxPlayersChanged();

private:
    void onNewSessions() override;
    void onClearAll() override;
    void onDataAdded() override;
    void onPOVChanged() override;
    void onQueriesChanged() override;
    void onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) override;
    void onQueriesApplied() override;

private:
    QComboBox* comboBox_you;
    QComboBox* comboBox_opp;
};
