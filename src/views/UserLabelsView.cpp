#include "decision-graph/views/UserLabelsView.hpp"
#include "decision-graph/models/UserLabelsModel.hpp"

#include <QComboBox>
#include <QCheckBox>
#include <QGridLayout>
#include <QTableWidget>
#include <QTabWidget>
#include <QLabel>
#include <QAbstractTableModel>

namespace {

class TableModel : public QAbstractTableModel
{
public:
    TableModel(UserLabelsModel* userLabels)
        : userLabels_(userLabels)
    {}

    int rowCount(const QModelIndex& parent) const override
    {
        return 3;
    }

    int columnCount(const QModelIndex& parent) const override
    {
        return 6;
    }

    QVariant data(const QModelIndex &index, int role) const override
    {
        switch (role) {
            case Qt::DisplayRole:
                if (index.row() == 0)
                {
                    if (index.column() == 0) return "uair";
                    if (index.column() == 1) return "attack_air_hi";
                    if (index.column() == 2) return "24923458345";
                    if (index.column() == 3) return "FIGHTER_STATUS_ATTACK_AIR";
                }
                break;

            case Qt::TextAlignmentRole:
                if (index.column() == 2) return Qt::AlignCenter;
                break;

            case Qt::CheckStateRole:
                if (index.row() == 0 && index.column() == 4) return Qt::Checked;
                if (index.row() == 0 && index.column() == 5) return Qt::Checked;
                break;
            case Qt::DecorationRole: break;
            case Qt::EditRole: break;
            case Qt::ToolTipRole: break;
            case Qt::StatusTipRole: break;
            case Qt::WhatsThisRole: break;
            case Qt::SizeHintRole: break;
        }

        return QVariant();
    }

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override
    {
        switch (role) {
            case Qt::DisplayRole:
                switch (orientation) {
                    case Qt::Horizontal:
                        if (section == 0) return "User Label";
                        if (section == 1) return "Motion Label";
                        if (section == 2) return "Motion";
                        if (section == 3) return "Status";
                        if (section == 4) return "Match Motion";
                        if (section == 5) return "Match Status";
                        break;

                    case Qt::Vertical:
                        return QString::number(section + 1);
                }
                break;
        }

        return QVariant();
    }

private:
    UserLabelsModel* userLabels_;
};

}

// ----------------------------------------------------------------------------
UserLabelsView::UserLabelsView(UserLabelsModel* model, QWidget* parent)
    : QDialog(parent)
    , model_(model)
{
    QGridLayout* layout = new QGridLayout;

    setWindowTitle("Edit User Labels");

    QLabel* fighterSelectLabel = new QLabel("Fighter:");
    layout->addWidget(fighterSelectLabel, 0, 0);

    QComboBox* fighterSelect = new QComboBox;
    fighterSelect->addItem("Pikachu");
    layout->addWidget(fighterSelect, 0, 1);

    QTabWidget* tabWidget = new QTabWidget;
    auto populateTabs = [this, &tabWidget](int idx, const char* name){
        tables_[idx] = new QTableView;
        tabWidget->addTab(tables_[idx], name);
    };
#define X(name, desc) populateTabs(static_cast<int>(UserLabelCategory::name), desc);
    USER_LABEL_CATEGORIES_LIST
#undef X
    layout->addWidget(tabWidget, 1, 0, 1, 3);

    layout->setColumnStretch(2, 1);
    setLayout(layout);

    tables_[static_cast<int>(UserLabelCategory::MOVEMENT)]->setModel(new TableModel(model_));

    model_->dispatcher.addListener(this);
}

// ----------------------------------------------------------------------------
UserLabelsView::~UserLabelsView()
{
    model_->dispatcher.removeListener(this);
    delete tables_[static_cast<int>(UserLabelCategory::MOVEMENT)]->model();
}
