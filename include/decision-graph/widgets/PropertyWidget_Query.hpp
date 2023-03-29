#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "decision-graph/widgets/PropertyWidget.hpp"

#include "rfcommon/Vector.hpp"

class QLineEdit;

class PropertyWidget_Query
        : public PropertyWidget
        , public SequenceSearchListener
{
    Q_OBJECT

public:
    PropertyWidget_Query(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_Query();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void addQueryBox();

private:
    void onLineEditQueryTextChanged(int index, const QString& text);

private:
    void onNewSessions() override;
    void onClearAll() override;
    void onDataAdded() override;
    void onPOVChanged() override;
    void onQueriesChanged() override;
    void onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError) override;
    void onQueriesApplied() override;

private:
    struct QueryBox
    {
        QLabel* name;
        QLabel* playerStatus;
        QLabel* opponentStatus;
        QLineEdit* playerQuery;
        QLineEdit* opponentQuery;
        QToolButton* remove;
    };

    QToolButton* toolButton_addQuery;
    rfcommon::Vector<QueryBox> queryBoxes_;
};
