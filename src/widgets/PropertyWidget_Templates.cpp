#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/widgets/PropertyWidget_Templates.hpp"

#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QPushButton>
#include <QListWidget>
#include <QStandardPaths>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>

// ----------------------------------------------------------------------------
PropertyWidget_Templates::PropertyWidget_Templates(SequenceSearchModel* model, QWidget* parent)
    : PropertyWidget(model, parent)
    , listWidget(new QListWidget)
{
    setTitle("Templates");

    QPushButton* pushButton_saveNew = new QPushButton("Save new");
    pushButton_saveNew->setIcon(QIcon::fromTheme("file"));

    QPushButton* pushButton_saveOver = new QPushButton("Save over");
    pushButton_saveOver->setIcon(QIcon::fromTheme("save"));

    QPushButton* pushButton_delete = new QPushButton("Delete");
    pushButton_delete->setIcon(QIcon::fromTheme("x"));

    QHBoxLayout* buttonsLayout = new QHBoxLayout;
    buttonsLayout->addWidget(pushButton_saveNew);
    buttonsLayout->addWidget(pushButton_saveOver);
    buttonsLayout->addWidget(pushButton_delete);

    QVBoxLayout* l = new QVBoxLayout;
    l->addWidget(listWidget);
    l->addLayout(buttonsLayout);

    contentWidget()->setLayout(l);
    updateSize();
}

// ----------------------------------------------------------------------------
PropertyWidget_Templates::~PropertyWidget_Templates()
{
}

// ----------------------------------------------------------------------------
void PropertyWidget_Templates::onPushButtonSaveNewReleased()
{
    QDir dir = getTemplateDir();
    QString templateName = findUnusedTemplateName(dir);
    if (saveToTemplate(dir.absoluteFilePath(templateName + ".json")) == false)
        return;

    QListWidgetItem* item = new QListWidgetItem(templateName);
    item->setFlags(item->flags() | Qt::ItemIsEditable);
    listWidget->addItem(item);
    listWidget->setCurrentItem(item);
    listWidget->editItem(item);
}

// ----------------------------------------------------------------------------
QDir PropertyWidget_Templates::getTemplateDir()
{
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation));
    if (dir.exists("decision-graph") == false)
        dir.mkdir("decision-graph");
    dir.cd("decision-graph");
    return dir;
}

// ----------------------------------------------------------------------------
QString PropertyWidget_Templates::findUnusedTemplateName(const QDir& dir)
{
    int number = 1;
    while (dir.exists("Template " + QString::number(number) + ".json"))
        number++;
    return "Template " + QString::number(number);
}

// ----------------------------------------------------------------------------
bool PropertyWidget_Templates::saveToTemplate(const QString& filePath)
{
    QJsonArray jQueries;
    for (int queryIdx = 0; queryIdx != seqSearchModel_->queryCount(); ++queryIdx)
        jQueries.push_back(QJsonObject({
            {"player", seqSearchModel_->playerQuery(queryIdx).cStr()},
            {"opponent", seqSearchModel_->opponentQuery(queryIdx).cStr()}
        }));

    QJsonObject jConstraints
    {
    };

    QJsonObject jTimingSettings
    {
        {"first", 0},
        {"second", 0}
    };

    QJsonObject jDamageSettings
    {
        {"bucketsize", 20}
    };

    QJsonObject jRoot
    {
        {"queries", jQueries},
        {"constraints", jConstraints},
        {"timings", jTimingSettings},
        {"damage", jDamageSettings}
    };

    QFile f(filePath);
    if (f.open(QIODevice::WriteOnly) == false)
    {
        QMessageBox::critical(this, "Error", "Failed to create file \"" + filePath + "\": " + f.errorString());
        return false;
    }

    f.write(QJsonDocument(jRoot).toJson());
    return true;
}

// ----------------------------------------------------------------------------
bool PropertyWidget_Templates::loadTemplate(const QString& filePath)
{
    QFile f(filePath);
    if (f.open(QIODevice::ReadOnly) == false)
    {
        QMessageBox::critical(this, "Error", "Failed to open file \"" + filePath + "\": " + f.errorString());
        return false;
    }

    QJsonObject jRoot = QJsonDocument::fromJson(f.readAll()).object();


    return true;
}
