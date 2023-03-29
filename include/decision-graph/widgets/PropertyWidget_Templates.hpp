#pragma once

#include "decision-graph/widgets/PropertyWidget.hpp"

#include <QDir>

class QListWidget;

class PropertyWidget_Templates : public PropertyWidget
{
    Q_OBJECT

public:
    PropertyWidget_Templates(SequenceSearchModel* model, QWidget* parent=nullptr);
    ~PropertyWidget_Templates();

    QVector<QWidget*> scrollIgnoreWidgets() override { return {}; }

private slots:
    void onPushButtonSaveNewReleased();

private:
    static QDir getTemplateDir();
    static QString findUnusedTemplateName(const QDir& dir);
    bool saveToTemplate(const QString& filePath);
    bool loadTemplate(const QString& filePath);

private:
    QListWidget* listWidget;
};
