#include "customwidgets.h"
#include <KAction>
#include <KStandardAction>
#include <kiconloader.h>
#include <cmath>
#include <QHash>

#define UNIVERSAL_PADDING 20
#define SEARCH_DELAY 300
#define ICON_WIDGET_HEIGHT 100
#define ICON_WIDGET_WIDTH 100

//AppletsList

using namespace KCategorizedItemsViewModels;

AppletsList::AppletsList(QGraphicsItem *parent)
        :QGraphicsWidget(parent)
{
    init();
    arrowClickStep = 0;
    scrollStep = 0;
    connect(this, SIGNAL(listScrolled()), this, SLOT(manageArrows()));
}

AppletsList::~AppletsList()
{
}

void AppletsList::init()
{
    m_leftArrow = new Plasma::PushButton();
    m_leftArrow->nativeWidget()->setIcon(KIcon("arrow-left"));
    m_leftArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));
    m_leftArrow->setAttribute(Qt::WA_NoSystemBackground);

    m_rightArrow = new Plasma::PushButton();
    m_rightArrow->nativeWidget()->setIcon(KIcon("arrow-right"));
    m_rightArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));

    connect(m_rightArrow, SIGNAL(clicked()), this, SLOT(onRightArrowClick()));
    connect(m_leftArrow, SIGNAL(clicked()), this, SLOT(onLeftArrowClick()));

    m_appletsListWidget = new QGraphicsWidget();
    m_appletListLinearLayout = new QGraphicsLinearLayout(Qt::Horizontal);
    m_appletsListWidget->setLayout(m_appletListLinearLayout);

    m_appletsListWindowWidget = new QGraphicsWidget();
    m_appletsListWindowWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    m_appletsListWidget->setParentItem(m_appletsListWindowWidget);

    m_arrowsLayout = new QGraphicsLinearLayout();

    m_arrowsLayout->addItem(m_leftArrow);
    m_arrowsLayout->addItem(m_appletsListWindowWidget);
    m_arrowsLayout->addItem(m_rightArrow);

    m_arrowsLayout->setAlignment(m_rightArrow, Qt::AlignRight);
    m_arrowsLayout->setAlignment(m_leftArrow, Qt::AlignLeft);

    m_arrowsLayout->setAlignment(m_rightArrow, Qt::AlignVCenter);
    m_arrowsLayout->setAlignment(m_leftArrow, Qt::AlignVCenter);

    setLayout(m_arrowsLayout);

}

void AppletsList::resizeEvent(QGraphicsSceneResizeEvent *event) {
    Q_UNUSED(event)
    m_appletsListWindowWidget->setMinimumHeight(m_appletsListWidget->geometry().height());
    m_appletsListWindowWidget->setMaximumHeight(m_appletsListWidget->geometry().height());

    arrowClickStep = ceil(m_appletsListWindowWidget->geometry().width()/6);
    scrollStep = ceil(m_appletsListWindowWidget->geometry().width()/2);
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
    m_searchString = text;
    m_searchDelayTimer.start(SEARCH_DELAY, this);
}

void AppletsList::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_searchDelayTimer.timerId()) {
        m_modelFilterItems->setSearch(m_searchString);
        m_searchDelayTimer.stop();
    }
    QGraphicsWidget::timerEvent(event);
}

void AppletsList::insertAppletIcon(AppletIconWidget *appletIconWidget)
{
    appletIconWidget->setVisible(true);
    m_appletListLinearLayout->addItem(appletIconWidget);
}

double AppletsList::listWidth() {
    return m_appletsListWidget->childItems().count() *
                    (ICON_WIDGET_WIDTH + m_appletListLinearLayout->spacing()) -
                    m_appletListLinearLayout->spacing();
}

AppletIconWidget *AppletsList::createAppletIcon(PlasmaAppletItem *appletItem)
{
    AppletIconWidget *applet = new AppletIconWidget();
    applet->setAppletItem(appletItem);
    applet->setIcon(appletItem->icon());
    applet->setText(appletItem->name());
//    ***** set this another way ********
    applet->setMinimumSize(ICON_WIDGET_WIDTH, ICON_WIDGET_HEIGHT);
    applet->setMaximumSize(ICON_WIDGET_WIDTH, ICON_WIDGET_HEIGHT);

    connect(applet, SIGNAL(hoverEnter(AppletIconWidget*)), this, SLOT(appletIconEnter(AppletIconWidget*)));

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

    m_appletsListWidget->setLayout(NULL);
    m_appletListLinearLayout = new QGraphicsLinearLayout();

    eraseList();

    for(int i = 0; i < m_modelFilterItems->rowCount(); i++) {
        item = getItemByProxyIndex(m_modelFilterItems->index(i, 0));
        appletItem = (PlasmaAppletItem*) item;

        if(appletItem != 0) {
            appletIconWidget = m_allAppletsHash->value(appletItem->id());
            insertAppletIcon(appletIconWidget);
        }
    }

    m_appletsListWidget->setLayout(m_appletListLinearLayout);
    m_appletListLinearLayout->setSpacing(10);
    resetScroll();
}

void AppletsList::wheelEvent(QGraphicsSceneWheelEvent *event) {
    if(event->delta() < 0) {
        scroll(true, true);
    } else {
        scroll(false, true);
    }
}

void AppletsList::onRightArrowClick() {
    scroll(true, false);
}

void AppletsList::onLeftArrowClick() {
    scroll(false, false);
}

void AppletsList::scroll(bool right, bool byWheel) {
    QRectF visibleRect = visibleListRect();
    int step = byWheel ? scrollStep : arrowClickStep;

    if(right) {
        scrollRight(step, visibleRect);
    } else {
        scrollLeft(step, visibleRect);
    }

}

void AppletsList::scrollRight(int step, QRectF visibleRect) {

    int lastVisibleXOnList = visibleRect.x() + visibleRect.width();
    int listWidthVar = listWidth();
    QRectF newVisibleRect;

    if(lastVisibleXOnList < listWidthVar) {
        if(lastVisibleXOnList + step > listWidthVar) {
            step = listWidthVar - lastVisibleXOnList;
        }

        m_appletsListWidget->moveBy(-step, 0);
        newVisibleRect = visibleListRect();
        scrollRightToShowClippedAppletIcon(newVisibleRect.x() + visibleRect.width());
        emit(listScrolled());
    }
}

void AppletsList::scrollLeft(int step, QRectF visibleRect) {

    int firstVisibleXOnList = visibleRect.x();
    QRectF newVisibleRect;

    if(firstVisibleXOnList > 0) {
        if(firstVisibleXOnList - step < 0 ) {
            step = firstVisibleXOnList;
        }

        m_appletsListWidget->moveBy(step, 0);
        newVisibleRect = visibleListRect();
        scrollLeftToShowClippedAppletIcon(newVisibleRect.x());
        emit(listScrolled());
    }
}

void AppletsList::scrollRightToShowClippedAppletIcon(int lastVisibleXOnList) {
    AppletIconWidget *clipped;
    int newFirstVisibleXOnList;
    clipped = findAppletUnderXPosition(lastVisibleXOnList);
    if(clipped) {
        newFirstVisibleXOnList = (clipped->pos().x() +
                                     clipped->boundingRect().width()) -
                                    m_appletsListWindowWidget->boundingRect().width();

        m_appletsListWidget->moveBy(-1 * (m_appletsListWidget->mapToItem(m_appletsListWindowWidget,
                                                                   newFirstVisibleXOnList, 0)).x() , 0);
    }
}

void AppletsList::scrollLeftToShowClippedAppletIcon(int firstVisibleXOnList) {
    AppletIconWidget *clipped;
    clipped = findAppletUnderXPosition(firstVisibleXOnList);
    if(clipped) {
        m_appletsListWidget->moveBy(-1 * (m_appletsListWidget->mapToItem(m_appletsListWindowWidget,
                                                                   clipped->pos().x(), 0)).x() , 0);
    }

}

void AppletsList::resetScroll() {
    m_appletsListWidget->setPos(0,0);
    manageArrows();
}

void AppletsList::manageArrows() {
    QRectF visibleRect;
    int firstVisibleXOnList;
    int lastVisibleXOnList;

    double listWidthVar = listWidth();

    if(listWidthVar <= m_appletsListWindowWidget->geometry().width()) {
        m_leftArrow->setEnabled(false);
        m_rightArrow->setEnabled(false);

    } else {
        visibleRect = visibleListRect();
        firstVisibleXOnList = visibleRect.x();
        lastVisibleXOnList = firstVisibleXOnList + visibleRect.width();

        if(firstVisibleXOnList <= 0) {
            m_leftArrow->setEnabled(false);
        } else {
            m_leftArrow->setEnabled(true);
        }

        if(lastVisibleXOnList >= listWidthVar) {
            m_rightArrow->setEnabled(false);
        } else {
            m_rightArrow->setEnabled(true);
        }
    }
}

AppletIconWidget *AppletsList::findAppletUnderXPosition(int xPosition) {
    QList<QGraphicsItem *> applets = m_appletsListWidget->childItems();

    int firstXOnApplet;
    int lastXOnApplet;

    foreach(QGraphicsItem *applet, applets) {
        firstXOnApplet = applet->mapToItem(m_appletsListWidget, 0, 0).x();
        lastXOnApplet = applet->mapToItem(m_appletsListWidget, applet->boundingRect().width(), 0).x();
        if((xPosition >= firstXOnApplet) && (xPosition <= lastXOnApplet)) {
            return dynamic_cast<AppletIconWidget*>(applet);
        }
    }
    return 0;
}

QRectF AppletsList::visibleListRect() {
    QRectF visibleRect = m_appletsListWindowWidget->
                         mapRectToItem(m_appletsListWidget, 0, 0,
                                          m_appletsListWindowWidget->geometry().width(),
                                          m_appletsListWindowWidget->geometry().height());

    return visibleRect;
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

void AppletsList::appletIconEnter(AppletIconWidget *appletIcon)
{
    emit(appletIconEnter(appletIcon->appletItem()));
}

QList <AbstractItem *> AppletsList::selectedItems() const
{
//    return m_appletList->selectedItems();
    return QList<AbstractItem *>();
}

//AppletIconWidget

AppletIconWidget::AppletIconWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem)
    : Plasma::IconWidget(parent)
{
    m_appletItem = appletItem;
    selected = false;
    m_selectedBackgroundSvg = new Plasma::FrameSvg(this);
    m_selectedBackgroundSvg->setImagePath("widgets/translucentbackground");

    if(m_appletItem != 0) {
        setText(appletItem->name());
        setIcon(appletItem->icon());
    } else {
        setText("no name");
        setIcon("widgets/clock");
    }
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

void AppletIconWidget::updateApplet(PlasmaAppletItem *appletItem)
{
    m_appletItem = appletItem;
    setIcon(m_appletItem->icon());
    setText(m_appletItem->name());
}

void AppletIconWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Plasma::IconWidget::hoverEnterEvent(event);
    selected = true;
    emit(hoverEnter(this));
}

void AppletIconWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Plasma::IconWidget::hoverLeaveEvent(event);
    selected = false;
    emit(hoverLeave(this));
}

void AppletIconWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
    if(selected) {
        m_selectedBackgroundSvg->resizeFrame(contentsRect().size());
        m_selectedBackgroundSvg->paintFrame(painter, contentsRect().topLeft());
     }

    Plasma::IconWidget::paint(painter, option, widget);
 }

//FilteringList

FilteringList::FilteringList(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
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
    m_treeView->nativeWidget()->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_treeView->nativeWidget()->setRootIsDecorated(false);
    m_treeView->nativeWidget()->setAttribute(Qt::WA_TranslucentBackground);
    m_treeView->setAttribute(Qt::WA_TranslucentBackground);
    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout();
    linearLayout->addItem(m_treeView);
    setLayout(linearLayout);
    m_treeView->nativeWidget()->header()->setVisible(false);
    m_treeView->nativeWidget()->setPalette((new Plasma::IconWidget())->palette());
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

//FilteringWidget

FilteringWidget::FilteringWidget(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
{
    m_backgroundSvg = new Plasma::FrameSvg(this);
    m_backgroundSvg->setImagePath("widgets/background");

    init();
}

FilteringWidget::~FilteringWidget(){
}

void FilteringWidget::init()
{
    m_filterLabel = new Plasma::Label();
    m_filterLabel->nativeWidget()->setText("Filter");
    m_filterLabel->setMaximumHeight(10);
    m_filterLabel->setMinimumHeight(10);

    QFont labelFont = m_filterLabel->font();
    labelFont.setPointSize(3);
    m_filterLabel->setFont(labelFont);

    m_textSearch = new Plasma::LineEdit();
    m_textSearch->nativeWidget()->setClickMessage(/*i18n(*/"Type search"/*)*/);
    m_textSearch->setFocus();
    m_textSearch->setAttribute(Qt::WA_NoSystemBackground);
    m_textSearch->setMaximumHeight(30);
    m_textSearch->setMinimumHeight(30);

    m_categoriesList = new FilteringList();

    QGraphicsLinearLayout *linearLayout = new QGraphicsLinearLayout(Qt::Vertical);

    linearLayout->addItem(m_filterLabel);
    linearLayout->addItem(m_textSearch);
    linearLayout->addItem(m_categoriesList);

    this->setLayout(linearLayout);
    linearLayout->setContentsMargins(15,15,15,15);
}

void FilteringWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
     QGraphicsWidget::paint(painter, option, widget);
     m_backgroundSvg->resizeFrame(contentsRect().size());
     m_backgroundSvg->paintFrame(painter, contentsRect().topLeft());
 }

FilteringList *FilteringWidget::categoriesList()
{
    return m_categoriesList;
}

Plasma::LineEdit *FilteringWidget::textSearch()
{
    return m_textSearch;
}

//ManageWidgetsPushButton

ManageWidgetsPushButton::ManageWidgetsPushButton(QGraphicsItem * parent, Qt::WindowFlags wFlags)
    : QGraphicsWidget(parent, wFlags)
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

