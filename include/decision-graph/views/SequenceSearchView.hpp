#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include "rfcommon/Vector.hpp"
#include <QWidget>
#include <memory>

namespace rfcommon {
    class MotionLabels;
}

// Forward declare the class created by Qt designer
namespace Ui {
    class SequenceSearchView;
}

class QLabel;
class QLineEdit;
class QToolButton;

class GraphModel;
class SequenceSearchModel;

class SequenceSearchView
        : public QWidget
        , public SequenceSearchListener
{
    Q_OBJECT

public:
    explicit SequenceSearchView(
            SequenceSearchModel* model,
            GraphModel* graphModel,
            rfcommon::MotionLabels* labels,
            QWidget* parent=nullptr);
    ~SequenceSearchView();

private slots:
    void onLineEditQueryTextChanged(int index, const QString& text);
    void onComboBoxPlayerChanged(int index);
    void addQueryBox();
    void removeQueryBox(int index);

private:
    void updateQueryCompileError(int queryIdx);

private:
    void onCurrentFighterChanged() override;
    void onNewSession() override;
    void onDataAdded() override;
    void onDataCleared() override;
    void onQueryCompiled(int queryIdx) override;
    void onQueryApplied() override;

private:
    struct QueryBox
    {
        QLabel* name;
        QLabel* parseError;
        QLineEdit* query;
        QToolButton* remove;
    };

    SequenceSearchModel* seqSearchModel_;
    GraphModel* graphModel_;
    Ui::SequenceSearchView* ui_;
    rfcommon::Vector<QueryBox> queryBoxes_;
};
