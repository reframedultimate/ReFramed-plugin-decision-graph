#pragma once

#include "decision-graph/listeners/UserLabelsListener.hpp"
#include "decision-graph/models/UserLabelCategory.hpp"
#include <QDialog>

class QTableView;
class UserLabelsModel;

class UserLabelsView : public QDialog
                     , public UserLabelsListener
{
    Q_OBJECT

public:
    explicit UserLabelsView(UserLabelsModel* model, QWidget* parent=nullptr);
    ~UserLabelsView();

private:
    UserLabelsModel* model_;
    QTableView* tables_[static_cast<int>(UserLabelCategory::COUNT)];
};
