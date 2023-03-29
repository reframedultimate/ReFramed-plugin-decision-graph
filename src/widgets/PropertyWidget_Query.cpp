#include "decision-graph/widgets/PropertyWidget_Query.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"

#include <QGridLayout>
#include <QLabel>
#include <QComboBox>
#include <QToolButton>
#include <QLineEdit>

// ----------------------------------------------------------------------------
static void clearLayout(QLayout* layout)
{
    QLayoutItem* item;
    while ((item = layout->takeAt(0)) != nullptr)
    {
        if (item->widget())
        {
            delete item->widget();
            delete item;
            continue;
        }
        if (item->layout())
            delete item;
    }
}

// ----------------------------------------------------------------------------
PropertyWidget_Query::PropertyWidget_Query(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
    , toolButton_addQuery(new QToolButton)
{
    setTitle("Query");

    toolButton_addQuery->setIcon(QIcon::fromTheme("plus"));

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(toolButton_addQuery);
    contentWidget()->setLayout(l);
    addQueryBox();

    seqSearchModel_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
PropertyWidget_Query::~PropertyWidget_Query()
{
    seqSearchModel_->dispatcher.removeListener(this);
}

// ----------------------------------------------------------------------------
void PropertyWidget_Query::addQueryBox()
{
    toolButton_addQuery->setParent(nullptr);

    QueryBox& box = queryBoxes_.emplace();
    box.name = new QLabel("#" + QString::number(queryBoxes_.count()) + ":");
    box.playerStatus = new QLabel("Query string empty.");
    box.playerStatus->setStyleSheet("QLabel {color: #FF2020}");
    box.opponentStatus = new QLabel;
    box.opponentStatus->setVisible(false);
    box.playerQuery = new QLineEdit;
    box.playerQuery->setPlaceholderText("You");
    box.opponentQuery = new QLineEdit;
    box.opponentQuery->setPlaceholderText("Opponent");
    box.remove = new QToolButton;
    box.remove->setIcon(QIcon::fromTheme("x"));

    QGridLayout* boxLayout = new QGridLayout;
    boxLayout->addWidget(box.name, 0, 0);
    boxLayout->addWidget(box.playerQuery, 0, 1);
    boxLayout->addWidget(box.remove, 0, 2);
    boxLayout->addWidget(box.playerStatus, 1, 0, 1, 2, Qt::AlignRight);
    boxLayout->addWidget(box.opponentQuery, 2, 1);
    boxLayout->addWidget(box.opponentStatus, 3, 0, 1, 2, Qt::AlignRight);

    QVBoxLayout* groupLayout = static_cast<QVBoxLayout*>(contentWidget()->layout());
    groupLayout->addLayout(boxLayout);
    groupLayout->addWidget(toolButton_addQuery, 0, Qt::AlignLeft);

    // Update tab order to be more convenient
    QWidget::setTabOrder(queryBoxes_[0].playerQuery, queryBoxes_[0].opponentQuery);
    for (int i = 1; i < queryBoxes_.count(); ++i)
    {
        QWidget::setTabOrder(queryBoxes_[i-1].opponentQuery, queryBoxes_[i].playerQuery);
        QWidget::setTabOrder(queryBoxes_[i].playerQuery, queryBoxes_[i].opponentQuery);
    }
    QWidget::setTabOrder(queryBoxes_.back().opponentQuery, toolButton_addQuery);
    for (int i = 1; i < queryBoxes_.count(); ++i)
        QWidget::setTabOrder(queryBoxes_[i-1].remove, queryBoxes_[i].remove);

    // Focus player query
    box.playerQuery->setFocus();

    updateSize();

    // Because the position of the query box might change in the UI later,
    // we use one of the contained widgets to search for the current index of
    // the box. Could be any widget, we choose the name.
    QLabel* nameWidget = box.name;
    auto findBoxIndex = [this, nameWidget]() -> int {
        for (int i = 0; i != queryBoxes_.count(); ++i)
            if (queryBoxes_[i].name == nameWidget)
                return i;
        assert(false);
        return -1;
    };

    connect(box.playerQuery, &QLineEdit::textChanged, [this, findBoxIndex](const QString& text) {
        onLineEditQueryTextChanged(findBoxIndex(), text);
    });
    connect(box.opponentQuery, &QLineEdit::textChanged, [this, findBoxIndex](const QString& text) {
        onLineEditQueryTextChanged(findBoxIndex(), text);
    });
    connect(box.remove, &QToolButton::released, [this, findBoxIndex] {
        int index = findBoxIndex();

        // Remove boxlayout from outer layout and delete everything
        QLayoutItem* boxLayout = contentWidget()->layout()->takeAt(index);
        clearLayout(boxLayout->layout());
        delete boxLayout;

        // Remove widgets from array and remove query from search model
        queryBoxes_.erase(index);
        seqSearchModel_->removeQuery(index);

        // Adjust query names
        for (; index != queryBoxes_.count(); ++index)
            queryBoxes_[index].name->setText("#" + QString::number(index + 1) + ":");

        // Don't have to recompile queries, but do have to apply all again
        if (seqSearchModel_->applyAllQueries())
            seqSearchModel_->notifyQueriesApplied();
    });

    seqSearchModel_->addQuery();
}

// ----------------------------------------------------------------------------
void PropertyWidget_Query::onLineEditQueryTextChanged(int index, const QString& text)
{
    QueryBox& box = queryBoxes_[index];

    if (box.playerQuery->text().length() == 0)
    {
        box.playerStatus->setText("Query string empty.");
        box.playerStatus->setStyleSheet("QLabel {color: #FF2020}");
        box.playerStatus->setVisible(true);
        updateSize();
        return;
    }

    seqSearchModel_->setQuery(index,
        box.playerQuery->text().toUtf8().constData(),
        box.opponentQuery->text().toUtf8().constData());
    seqSearchModel_->notifyQueriesChanged();

    if (seqSearchModel_->compileQuery(index))
        if (seqSearchModel_->applyQuery(index))
            seqSearchModel_->notifyQueriesApplied();
}

// ----------------------------------------------------------------------------
void PropertyWidget_Query::onNewSessions() {}
void PropertyWidget_Query::onClearAll() {}
void PropertyWidget_Query::onDataAdded() {}
void PropertyWidget_Query::onPOVChanged() {}
void PropertyWidget_Query::onQueriesChanged() {}
void PropertyWidget_Query::onQueryCompiled(int queryIdx, bool success, const char* error, bool oppSuccess, const char* oppError)
{
    queryBoxes_[queryIdx].playerStatus->setVisible(!success);
    queryBoxes_[queryIdx].playerStatus->setText(QString::fromUtf8(error));

    queryBoxes_[queryIdx].opponentStatus->setVisible(!oppSuccess);
    queryBoxes_[queryIdx].opponentStatus->setText(QString::fromUtf8(oppError));

    updateSize();
}
void PropertyWidget_Query::onQueriesApplied() {}
