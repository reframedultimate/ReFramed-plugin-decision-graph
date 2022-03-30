#pragma once

#include "decision-graph/listeners/IncrementalDataListener.hpp"
#include <QWidget>

// Forward declare the class created by Qt designer
namespace Ui {
    class SequenceSearchView;
}

class IncrementalData;

class SequenceSearchView : public QWidget
                         , public IncrementalDataListener
{
    Q_OBJECT

public:
    explicit SequenceSearchView(IncrementalData* builder, QWidget* parent=nullptr);
    ~SequenceSearchView();

private:
    void onIncrementalDataNewStats() override;

private:
    IncrementalData* incData_;
    Ui::SequenceSearchView* ui_;
};

