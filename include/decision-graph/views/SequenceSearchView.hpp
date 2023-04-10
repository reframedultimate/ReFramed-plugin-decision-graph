#pragma once

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
class RegionScene;
class SequenceSearchModel;

class SequenceSearchView : public QWidget
{
    Q_OBJECT

public:
    explicit SequenceSearchView(
            SequenceSearchModel* model,
            GraphModel* graphModel,
            RegionScene* regionModel,
            rfcommon::MotionLabels* labels,
            QWidget* parent=nullptr);
    ~SequenceSearchView();

private:
    SequenceSearchModel* seqSearchModel_;
    Ui::SequenceSearchView* ui_;
};
