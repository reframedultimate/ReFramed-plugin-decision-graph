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
class QDir;

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
    void onTabIndexChanged(int index);
    void onLineEditQueryTextChanged(int index, const QString& text);
    void addQueryBox();
    void onPushButtonSaveNewReleased();

private:
    void onComboBoxPlayersChanged();
    static QDir getTemplateDir();
    static QString findUnusedTemplateName(const QDir& dir);
    bool saveToTemplate(const QString& filePath);
    bool loadTemplate(const QString& filePath);

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

    SequenceSearchModel* seqSearchModel_;
    GraphModel* graphModel_;
    Ui::SequenceSearchView* ui_;
    rfcommon::Vector<QueryBox> queryBoxes_;
};
