#pragma once

#include "decision-graph/listeners/GraphBuilderListener.hpp"
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
    void onGraphBuilderNewStats() override;

private:
    IncrementalData* builder_;
    Ui::SequenceSearchView* ui_;
};

