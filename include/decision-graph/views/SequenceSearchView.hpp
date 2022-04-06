#pragma once

#include "decision-graph/listeners/SequenceSearchListener.hpp"
#include <QWidget>
#include <memory>

// Forward declare the class created by Qt designer
namespace Ui {
    class SequenceSearchView;
}

class GraphModel;
class SequenceSearchModel;
class UserLabelsModel;

class SequenceSearchView : public QWidget
                         , public SequenceSearchListener
{
    Q_OBJECT

public:
    explicit SequenceSearchView(
            SequenceSearchModel* model,
            GraphModel* graphModel,
            UserLabelsModel* userLabelsModel,
            QWidget* parent=nullptr);
    ~SequenceSearchView();

private slots:
    void onLineEditQueryTextChanged(const QString& text);
    void onComboBoxPlayerChanged(int index);
    void onPushButtonEditLabelsReleased();

private:
    void applyCurrentQuery();

private:
    void onSessionChanged() override;
    void onCurrentFighterChanged() override;
    void onSequenceChanged() override;
    void onQueryChanged() override;

private:
    SequenceSearchModel* model_;
    GraphModel* graphModel_;
    UserLabelsModel* userLabelsModel_;
    Ui::SequenceSearchView* ui_;
};
