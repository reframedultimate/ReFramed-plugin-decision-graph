#include "ui_SequenceSearchView.h"
#include "decision-graph/models/GraphModel.hpp"
#include "decision-graph/models/SequenceSearchModel.hpp"
#include "decision-graph/views/DamageView.hpp"
#include "decision-graph/views/GraphView.hpp"
#include "decision-graph/views/HeatMapView.hpp"
#include "decision-graph/views/PieChartView.hpp"
#include "decision-graph/views/SequenceSearchView.hpp"
#include "decision-graph/views/ShieldHealthView.hpp"
#include "decision-graph/views/StateListView.hpp"
#include "decision-graph/views/TimingsView.hpp"
#include "decision-graph/widgets/PropertyWidget_Damage.hpp"
#include "decision-graph/widgets/PropertyWidget_DamageConstraints.hpp"
#include "decision-graph/widgets/PropertyWidget_HeatMap.hpp"
#include "decision-graph/widgets/PropertyWidget_PositionConstraints.hpp"
#include "decision-graph/widgets/PropertyWidget_POV.hpp"
#include "decision-graph/widgets/PropertyWidget_Query.hpp"
#include "decision-graph/widgets/PropertyWidget_RelativeConstraints.hpp"
#include "decision-graph/widgets/PropertyWidget_Shield.hpp"
#include "decision-graph/widgets/PropertyWidget_ShieldConstraints.hpp"
#include "decision-graph/widgets/PropertyWidget_Templates.hpp"
#include "decision-graph/widgets/PropertyWidget_Timings.hpp"

#include <QLineEdit>
#include <QLabel>
#include <QToolButton>
#include <QVBoxLayout>
#include <QSpacerItem>
#include <QGuiApplication>
#include <QScreen>

// ----------------------------------------------------------------------------
SequenceSearchView::SequenceSearchView(
        SequenceSearchModel* model,
        GraphModel* graphModel,
        rfcommon::MotionLabels* labels,
        QWidget* parent)
    : QWidget(parent)
    , seqSearchModel_(model)
    , graphModel_(graphModel)
    , ui_(new Ui::SequenceSearchView)  // Instantiate UI created in QtDesigner
{
    // Set up UI created in QtDesigner
    ui_->setupUi(this);

    ui_->tab_stateList->setLayout(new QVBoxLayout);
    ui_->tab_stateList->layout()->addWidget(new StateListView(model, labels));

    ui_->tab_pieChart->setLayout(new QVBoxLayout);
    ui_->tab_pieChart->layout()->addWidget(new PieChartView(model, labels));

    graphModel_->addEllipse(0, 0, 10, 15);
    ui_->tab_graph->setLayout(new QVBoxLayout);
    ui_->tab_graph->layout()->addWidget(new GraphView(graphModel, model, labels));

    ui_->tab_timings->setLayout(new QVBoxLayout);
    ui_->tab_timings->layout()->addWidget(new TimingsView(model, labels));

    ui_->tab_damage->setLayout(new QVBoxLayout);
    ui_->tab_damage->layout()->addWidget(new DamageView(model, labels));

    ui_->tab_shield->setLayout(new QVBoxLayout);
    ui_->tab_shield->layout()->addWidget(new ShieldHealthView(model, labels));

    ui_->tab_heatmap->setLayout(new QVBoxLayout);
    ui_->tab_heatmap->layout()->addWidget(new HeatMapView(model, labels));

    PropertyWidget* pwPOV = new PropertyWidget_POV(seqSearchModel_);
    PropertyWidget* pwQuery = new PropertyWidget_Query(seqSearchModel_);
    PropertyWidget* pwTimings = new PropertyWidget_Timings(seqSearchModel_);
    PropertyWidget* pwDamage = new PropertyWidget_Damage(seqSearchModel_);
    PropertyWidget* pwShield = new PropertyWidget_Shield(seqSearchModel_);
    PropertyWidget* pwHeatMap = new PropertyWidget_HeatMap(seqSearchModel_);
    PropertyWidget* pwTemplates = new PropertyWidget_Templates(seqSearchModel_);

    QVBoxLayout* propertiesLayout = new QVBoxLayout;
    propertiesLayout->addWidget(pwPOV);
    propertiesLayout->addWidget(pwQuery);
    propertiesLayout->addWidget(pwTimings);
    propertiesLayout->addWidget(pwDamage);
    propertiesLayout->addWidget(pwShield);
    propertiesLayout->addWidget(pwHeatMap);
    propertiesLayout->addWidget(pwTemplates);
    propertiesLayout->addSpacerItem(new QSpacerItem(0, 0, QSizePolicy::Minimum, QSizePolicy::Expanding));

    // Fix initial splitter sizes before adding it
    ui_->splitter->setSizes(QList<int>({propertiesLayout->sizeHint().width() , ui_->tabWidget->sizeHint().width()}));
    ui_->scrollAreaWidgetContents->setLayout(propertiesLayout);

    pwPOV->setExpanded(true);
    pwQuery->setExpanded(true);
    pwTimings->setExpanded(true);
    pwDamage->setExpanded(true);
    pwShield->setExpanded(true);
    pwHeatMap->setExpanded(true);
    pwTemplates->setExpanded(true);

    connect(ui_->tabWidget, &QTabWidget::currentChanged, [pwTimings](int index) { pwTimings->setVisible(index == 3); });
    connect(ui_->tabWidget, &QTabWidget::currentChanged, [pwDamage] (int index) { pwDamage->setVisible (index == 4); });
    connect(ui_->tabWidget, &QTabWidget::currentChanged, [pwShield](int index)  { pwShield->setVisible (index == 5); });
    connect(ui_->tabWidget, &QTabWidget::currentChanged, [pwHeatMap](int index) { pwHeatMap->setVisible(index == 6); });

    ui_->tabWidget->setCurrentIndex(0);
}

// ----------------------------------------------------------------------------
SequenceSearchView::~SequenceSearchView()
{
    delete ui_;
}
