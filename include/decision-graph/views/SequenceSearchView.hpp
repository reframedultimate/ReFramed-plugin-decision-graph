#pragma once

#include "decision-graph/listeners/GraphBuilderListener.hpp"
#include <QWidget>

// Forward declare the class created by Qt designer
namespace Ui {
    class SequenceSearchView;
}

class GraphBuilder;

class SequenceSearchView : public QWidget
                         , public GraphBuilderListener
{
    Q_OBJECT

public:
    explicit SequenceSearchView(GraphBuilder* builder, QWidget* parent=nullptr);
    ~SequenceSearchView();

private:
    void onGraphBuilderNewStats() override;

private:
    GraphBuilder* builder_;
    Ui::SequenceSearchView* ui_;
};

