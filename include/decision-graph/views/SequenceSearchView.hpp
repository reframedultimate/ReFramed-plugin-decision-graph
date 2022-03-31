#pragma once

#include "decision-graph/listeners/IncrementalDataListener.hpp"
#include <QWidget>

// Forward declare the class created by Qt designer
namespace Ui {
    class SequenceSearchView;
}

class IncrementalData;
class MotionsTable;

class SequenceSearchView : public QWidget
                         , public IncrementalDataListener
{
    Q_OBJECT

public:
    explicit SequenceSearchView(IncrementalData* incData, MotionsTable* motionsTable, QWidget* parent=nullptr);
    ~SequenceSearchView();

private slots:
    void onLineEditQueryTextChanged(const QString& text);

private:
    void onIncrementalDataNewStats() override;

private:
    IncrementalData* incData_;
    MotionsTable* motionsTable_;
    Ui::SequenceSearchView* ui_;
};

