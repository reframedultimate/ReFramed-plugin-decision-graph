#pragma once

#include <QWidget>
#include <QVector>

class SequenceSearchModel;

class QToolButton;
class QParallelAnimationGroup;
class QLabel;

/*!
 * @brief Collapsible widget. Code was copied from here and adapted:
 * http://stackoverflow.com/questions/32476006/how-to-make-an-expandable-collapsable-section-widget-in-qt
 *
 * Derived classes should place their custom widges into contentWidget() and
 * then call updateSize() when done. If you are using UI forms then do:
 * ```cpp
 * ui->setupUi(contentWidget()); // In constructor
 * ui->retranslateUi(contentWidget()); // When retranslating
 * ```
 *
 * You should also call ```setExpanded(true);``` once you've finished
 * constructing, so your widget is initially expanded.
 */
class PropertyWidget
        : public QWidget
{
    Q_OBJECT

public:
    explicit PropertyWidget(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget();

    void setTitle(const QString& title);
    QWidget* contentWidget();

    /*!
     * \brief Derived classes should return a list of widgets that
     * should be ignored by the scroll wheel
     */
    virtual QVector<QWidget*> scrollIgnoreWidgets() = 0;

    bool isExpanded() const;

public slots:
    void setExpanded(bool expanded);
    void onToggleButtonClicked(bool checked);

protected:
    void updateSize();

    SequenceSearchModel* seqSearchModel_;

private:
    QToolButton* toggleButton_;
    QLabel* title_;
    QParallelAnimationGroup* toggleAnimation_;
    QWidget* contentArea_;
    int animationDuration_;
    int collapsedHeight_;
};
