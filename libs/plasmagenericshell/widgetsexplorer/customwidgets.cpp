#include "customwidgets.h"
#include <KAction>
#include <KStandardAction>
#include<kiconloader.h>
#include<cmath>
#include<QHash>

#define UNIVERSAL_PADDING 20

//AppletsListSearch

AppletsListSearch::AppletsListSearch(QGraphicsItem *parent)
        :StandardCustomWidget(parent)
{
    init();

}

AppletsListSearch::~AppletsListSearch()
{
}

void AppletsListSearch::init()
{
    m_textSearch = new KLineEdit();
    m_textSearch->setClickMessage(/*i18n(*/"Enter search phrase here"/*)*/);
    m_textSearch->setFocus();
    m_textSearch->setAttribute(Qt::WA_NoSystemBackground);

    m_textSearchProxy = new QGraphicsProxyWidget();
    m_textSearchProxy->setAttribute(Qt::WA_NoSystemBackground);
    m_textSearchProxy->setWidget(m_textSearch);

    m_textSearchProxy->setMaximumHeight(30);
    m_textSearchProxy->setMinimumHeight(30);

    connect(m_textSearch, SIGNAL(textChanged(QString)), this, SLOT(searchTermChanged(QString)));

    m_appletsList = new AppletsList();

    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    layout->addItem(m_textSearchProxy);

    layout->addItem(m_appletsList);

    QAction * find = KStandardAction::find(m_textSearch, SLOT(setFocus()), this);
    addAction(find);

//    connect(m_appletList, SIGNAL(doubleClicked(const QModelIndex &)), this, SIGNAL(doubleClicked(const QModelIndex &)));
//    connect(m_appletList, SIGNAL(clicked(AbstractItem *)), this, SIGNAL(clicked(AbstractItem *)));
    connect(m_appletsList, SIGNAL(appletIconHoverEnter(PlasmaAppletItem*)), this, SIGNAL(entered(PlasmaAppletItem *)));

    setLayout(layout);

}

void AppletsListSearch::setFilterModel(QStandardItemModel *model)
{
    m_appletsList->setFilterModel(model);
}

void AppletsListSearch::setItemModel(QStandardItemModel *model)
{
    //esse é o meu
    m_appletsList->setItemModel(model);
}

void AppletsListSearch::filterChanged(int index)
{
    m_appletsList->filterChanged(index);
}


void AppletsListSearch::searchTermChanged(const QString &text)
{
    m_appletsList->searchTermChanged(text);
}

QList <AbstractItem *> AppletsListSearch::selectedItems() const
{
//    return m_appletList->selectedItems();
}

//AppletsList

AppletsList::AppletsList(QGraphicsItem *parent)
        :StandardCustomWidget(parent)
{
    init();

}

AppletsList::~AppletsList()
{
}

void AppletsList::init()
{
    m_leftArrow = new Plasma::PushButton();
    m_leftArrow->nativeWidget()->setIcon(KIcon("arrow-left"));
    m_leftArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));

    m_rightArrow = new Plasma::PushButton();
    m_rightArrow->nativeWidget()->setIcon(KIcon("arrow-right"));
    m_rightArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));

    m_appletsListWidget = new QGraphicsWidget();
    m_appletListGridLayout = new QGraphicsGridLayout();
    m_appletsListWidget->setLayout(m_appletListGridLayout);

    m_arrowsLayout = new QGraphicsLinearLayout();

    m_arrowsLayout->addItem(m_leftArrow);
    m_arrowsLayout->addItem(m_appletsListWidget);
    m_arrowsLayout->addItem(m_rightArrow);

    setLayout(m_arrowsLayout);

}

void AppletsList::setItemModel(QStandardItemModel *model)
{
    m_modelFilterItems = new DefaultItemFilterProxyModel(this);

    m_modelItems = model;
    m_modelFilterItems->setSortCaseSensitivity(Qt::CaseInsensitive);
    m_modelFilterItems->setDynamicSortFilter(true);
    m_modelFilterItems->setSourceModel(m_modelItems);
    m_modelFilterItems->sort(0);

    populateAllAppletsHash();
    updateList();

    connect(m_modelFilterItems, SIGNAL(searchTermChanged(QString)), this, SLOT(updateList()));
    connect(m_modelFilterItems, SIGNAL(filterChanged()), this, SLOT(updateList()));

}

void AppletsList::setFilterModel(QStandardItemModel *model)
{
    m_modelFilters = model;
}

void AppletsList::filterChanged(int index)
{
    if (m_modelFilterItems) {
        QVariant data = m_modelFilters->item(index)->data();
        m_modelFilterItems->setFilter(qVariantValue<KCategorizedItemsViewModels::Filter>(data));
    }
}

void AppletsList::searchTermChanged(const QString &text)
{
    if (m_modelFilterItems) {
        m_modelFilterItems->setSearch(text);
    }
}

void AppletsList::insertAppletIcon(int row, int column, AppletIconWidget *appletIconWidget)
{
    appletIconWidget->setVisible(true);
    m_appletListGridLayout->addItem(appletIconWidget, row, column);
}

AppletIconWidget *AppletsList::createAppletIcon(PlasmaAppletItem *appletItem)
{
    AppletIconWidget *applet = new AppletIconWidget();
    applet->setAppletItem(appletItem);
    applet->setIcon(appletItem->icon());
    applet->setText(appletItem->name());
    applet->setMinimumSize(100, 100);
    applet->setMaximumSize(100, 100);

    connect(applet, SIGNAL(hoverEnter(AppletIconWidget*)), this, SLOT(appletIconHoverEnter(AppletIconWidget*)));

    return applet;
}

void AppletsList::eraseList() {
    QList<QGraphicsItem *> applets = m_appletsListWidget->childItems();
    foreach(QGraphicsItem *applet, applets) {
        applet->setParentItem(0);
        applet->setVisible(false);
    }
}

void AppletsList::updateList()
{
    AbstractItem *item;
    PlasmaAppletItem *appletItem;
    AppletIconWidget *appletIconWidget;

    int row = 0;
    int column = 0;

    //make a define out of this
    int gridMaximumColumnCount = 3;

    m_appletsListWidget->setLayout(NULL);
    m_appletListGridLayout = new QGraphicsGridLayout();

    eraseList();

    for(int i = 0; i < m_modelFilterItems->rowCount(); i++) {
        item = getItemByProxyIndex(m_modelFilterItems->index(i, 0));
        appletItem = (PlasmaAppletItem*) item;

        if(appletItem != 0) {
            appletIconWidget = m_allAppletsHash->value(appletItem->id());

            row = i/gridMaximumColumnCount;
            column = i%gridMaximumColumnCount;

            insertAppletIcon(row, column, appletIconWidget);
        }
    }

    m_appletListGridLayout->setSpacing(10);
    m_appletsListWidget->setLayout(m_appletListGridLayout);
}

void AppletsList::populateAllAppletsHash()
{
    AbstractItem *item;
    PlasmaAppletItem *appletItem;
    int indexesCount = m_modelFilterItems->rowCount();

    m_allAppletsHash = new QHash<QString, AppletIconWidget *>();

    for(int i = 0; i < indexesCount ; i++){
        item = getItemByProxyIndex(m_modelFilterItems->index(i, 0));
        appletItem = (PlasmaAppletItem*) item;
        m_allAppletsHash->insert(appletItem->id(), createAppletIcon(appletItem));
    }
}

AbstractItem *AppletsList::getItemByProxyIndex(const QModelIndex &index) const
{
    return (AbstractItem *)m_modelItems->itemFromIndex(m_modelFilterItems->mapToSource(index));
}

void AppletsList::appletIconHoverEnter(AppletIconWidget *appletIcon)
{
    emit(appletIconHoverEnter(appletIcon->appletItem()));
}

//AppletInfoWidget

AppletInfoWidget::AppletInfoWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem, QSizeF constSize)
        : StandardCustomWidget(parent), m_constSize(constSize)
{
    m_appletItem = appletItem;
    init();
}

AppletInfoWidget::~AppletInfoWidget()
{
}

void AppletInfoWidget::init()
{
    setMinimumSize(m_constSize);
    setMaximumSize(m_constSize);

    m_appletIconWidget = new AppletIconWidget(0, m_appletItem, false);
    m_appletIconWidget->setMaximumSize(QSizeF(m_constSize.width(), 2 * (m_constSize.height()/4)));
    m_appletIconWidget->setMinimumSize(QSizeF(m_constSize.width(), 2 * (m_constSize.height()/4)));
    m_appletIconWidget->setAcceptHoverEvents(false);
    m_appletIconWidget->setAcceptedMouseButtons(false);

    m_appletDescription = new Plasma::Label();
    m_appletDescription->setAlignment(Qt::AlignCenter);
//    m_appletDescription->setMinimumSize(QSizeF(contentsRect().size().width(), contentsRect().size().height()/4));
    m_appletDescription->setMinimumSize(QSizeF(m_constSize.width(), m_constSize.height()/4));
    m_appletDescription->setScaledContents(true);
    m_appletDescription->nativeWidget()->setWordWrap(true);

    if(m_appletItem != 0) {
        m_appletDescription->setText(m_appletItem->description());
    } else {
        m_appletDescription->setText("Applet description");
    }

    m_appletInfoButton = new Plasma::IconWidget;
    m_appletInfoButton->setIcon("help-about");
    //m_appletInfoButton->setMinimumSize(QSizeF(25, 25));
    m_appletInfoButton->setMaximumSize(QSizeF(25, 25));

    m_linearLayout = new QGraphicsLinearLayout;
    m_linearLayout->setOrientation(Qt::Vertical);
    m_linearLayout->setMinimumSize(m_constSize);

    m_linearLayout->addItem(m_appletIconWidget);
    m_linearLayout->addItem(m_appletDescription);
    m_linearLayout->addItem(m_appletInfoButton);

    setLayout(m_linearLayout);

    m_linearLayout->setAlignment(m_appletInfoButton, Qt::AlignRight);
    m_linearLayout->setAlignment(m_appletIconWidget, Qt::AlignCenter);
    m_linearLayout->setAlignment(m_appletDescription, Qt::AlignCenter);

    connect(m_appletInfoButton, SIGNAL(clicked()), this, SLOT(onInfoButtonClicked()));

    //m_linearLayout->setContentsMargins(UNIVERSAL_PADDING, UNIVERSAL_PADDING, UNIVERSAL_PADDING, UNIVERSAL_PADDING);

}

void AppletInfoWidget::updateApplet(PlasmaAppletItem *appletItem)
{
    if(appletItem != NULL) {
        m_appletItem = appletItem;
        m_appletIconWidget->updateApplet(m_appletItem);
        m_appletDescription->setText(m_appletItem->description());
    }
}

void AppletInfoWidget::onInfoButtonClicked()
{
    emit(infoButtonClicked(m_appletItem->id()));
}

//AppletIconWidget

AppletIconWidget::AppletIconWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem, bool dotsSurrounded)
    : Plasma::IconWidget(parent)
{

//    m_appletPositions = new QList<QPointF>();
//    m_appletPositions->append(QPointF(30, 40));
//    m_dotsSurrounded = dotsSurrounded;
//    m_appletPositions->append(new QPointF(30, 40));
//    m_appletPositions->append(new QPointF(30, 40));
//    m_appletPositions->append(new QPointF(30, 40));
//    m_positionsDots = NULL;

    m_appletItem = appletItem;

    if(m_appletItem != 0) {
        setText(appletItem->name());
        setIcon(appletItem->icon());
    } else {
        setText("no name");
        setIcon("widgets/clock");
    }

//    m_dotsSphereRadius = (boundingRect().height()/2) + 5;
//    m_angleBetweenDots = 5; //why 5?

}

AppletIconWidget::~AppletIconWidget()
{
    m_appletItem = 0;
}

PlasmaAppletItem *AppletIconWidget::appletItem()
{
    return m_appletItem;
}

void AppletIconWidget::setAppletItem(PlasmaAppletItem *appletIcon)
{
   m_appletItem = appletIcon;
}


void AppletIconWidget::setAngleBetweenDots(double angle)
{
    m_angleBetweenDots = angle;
}

void AppletIconWidget::placeDotsAroundIcon()
{
    double angleInterval = m_maxAnglePosition - m_minAnglePosition;
    int dotsCount = m_appletPositions->count();
    double angleFirstDot = m_minAnglePosition + (dotsCount/2 -1);
    if(dotsCount%2 == 0) {
        angleFirstDot += m_angleBetweenDots/2;
    }

    //foreach(QPointF position, m_appletPositions){
        PositionDotsSvgWidget *dot = new PositionDotsSvgWidget(this);

        dot->setPos(sin(angleFirstDot)/m_dotsSphereRadius, cos(angleFirstDot)/m_dotsSphereRadius);
        dot->setParentItem(this);
    //}

}

void AppletIconWidget::updateApplet(PlasmaAppletItem *appletItem)
{
    m_appletItem = appletItem;
    setIcon(m_appletItem->icon());
    setText(m_appletItem->name());
}

void AppletIconWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    //hoverEnterEvent(event);
    emit(hoverEnter(this));
}

void AppletIconWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
   // hoverLeaveEvent(event);
    emit(hoverLeave(this));
}

//PositionDotsSvgWidget

PositionDotsSvgWidget::PositionDotsSvgWidget(QGraphicsWidget *parent)
        : Plasma::IconWidget(parent)
{

    m_appletPosition = new QPointF(200, 30);
    setIcon("clock");
    m_anglePosition = 0;

}

//FilteringList

FilteringList::FilteringList(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : StandardCustomWidget(parent, wFlags)
{
    init();
    connect(m_treeView->nativeWidget(), SIGNAL(clicked(const QModelIndex &)), this, SLOT(filterChanged(const QModelIndex &)));
}

FilteringList::~FilteringList()
{
}

void FilteringList::init()
{
    m_treeView = new Plasma::TreeView();
    m_treeView->nativeWidget()->setAttribute(Qt::WA_NoSystemBackground);
    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout();
    linearLayout->addItem(m_treeView);
    this->setLayout(linearLayout);
    m_treeView->nativeWidget()->header()->setVisible(false);
}

void FilteringList::setModel(QStandardItemModel *model)
{
    m_model = model;
    m_treeView->setModel(m_model);
}


void FilteringList::filterChanged(const QModelIndex & index)
{
    emit(filterChanged(index.row()));
}

//ManageWidgetsPushButton

ManageWidgetsPushButton::ManageWidgetsPushButton(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : StandardCustomWidget(parent, wFlags)
{
    init();
}

ManageWidgetsPushButton::~ManageWidgetsPushButton(){
}


void ManageWidgetsPushButton::init()
{
    m_button = new KPushButton();
    m_button->setFlat(true);
    m_button->setAttribute(Qt::WA_NoSystemBackground);
    m_buttonProxy = new QGraphicsProxyWidget();
    m_buttonProxy->setWidget(m_button);
    QGraphicsLinearLayout *layout = new QGraphicsLinearLayout(Qt::Vertical);
    layout->addItem(m_buttonProxy);
    layout->setAlignment(m_buttonProxy, Qt::AlignVCenter);
    setLayout(layout);

}

KPushButton *ManageWidgetsPushButton::button()
{
    return m_button;
}

KMenu *ManageWidgetsPushButton::buttonMenu()
{
    return m_buttonMenu;
}


QGraphicsProxyWidget *ManageWidgetsPushButton::buttonProxy()
{
    return m_buttonProxy;
}

