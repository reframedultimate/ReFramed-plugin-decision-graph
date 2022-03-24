#pragma once

#include <QWidget>

// Forward declare the class created by Qt designer
namespace Ui {
    class SequenceSearchView;
}


class SequenceSearchView : public QWidget
{
    Q_OBJECT

public:
    explicit SequenceSearchView(QWidget* parent=nullptr);
    ~SequenceSearchView();

private:
    Ui::SequenceSearchView* ui_;
};

