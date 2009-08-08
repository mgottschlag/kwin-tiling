#include "appletslist.h"
#include "widgetexplorer.h"

#include <cmath>

#include <kiconloader.h>
#include <kicon.h>
#include <kpushbutton.h>

#include <plasma/corona.h>

#include <QHash>

#define FILTER_APPLIANCE_DELAY 400
#define SEARCH_DELAY 300
#define TOOLTIP_APPEAR_DELAY 1000
#define TOOLTIP_DISAPPEAR_DELAY 300

#define ICON_SIZE 70
#define TOOLTIP_HEIGHT 200
#define TOOLTIP_WIDTH 200

using namespace KCategorizedItemsViewModels;

AppletsList::AppletsList(QGraphicsItem *parent)
        :QGraphicsWidget(parent)
{
    init();
    arrowClickStep = 0;
    scrollStep = 0;
    m_selectedItem = 0;
    m_currentAppearingAppletsOnList = new QList<AppletIconWidget *>();
    connect(this, SIGNAL(listScrolled()), this, SLOT(manageArrows()));

    scrollTimeLine.setFrameRange(0, 100);
    scrollTimeLine.setCurveShape(QTimeLine::EaseInOutCurve);
    scrollTimeLine.setDuration(500); // TODO: Set this to a lesser value
    connect(&scrollTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(scrollTimeLineFrameChanged(int)));

    m_toolTip = new AppletToolTipWidget();
    m_toolTip->setVisible(false);
    connect(m_toolTip, SIGNAL(enter()), this, SLOT(onToolTipEnter()));
    connect(m_toolTip, SIGNAL(leave()), this, SLOT(onToolTipLeave()));
    connect(m_toolTip, SIGNAL(infoButtonClicked(QString)), this, SIGNAL(infoButtonClicked(QString)));
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

    int maxVisibleIconsOnList = maximumAproxVisibleIconsOnList();
    arrowClickStep = ceil(maxVisibleIconsOnList/4);
    scrollStep = ceil(maxVisibleIconsOnList/2);
    manageArrows();
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

    connect(m_modelFilterItems, SIGNAL(searchTermChanged(QString)), this, SLOT(updateList()));
    connect(m_modelFilterItems, SIGNAL(filterChanged()), this, SLOT(updateList()));

    updateList();

}

void AppletsList::setFilterModel(QStandardItemModel *model)
{
    m_modelFilters = model;
}

void AppletsList::filterChanged(int index)
{
    if (m_modelFilterItems) {
        m_dataFilterAboutToApply = m_modelFilters->item(index)->data();
        m_filterApplianceTimer.start(FILTER_APPLIANCE_DELAY, this);
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

    if(event->timerId() == m_toolTipAppearTimer.timerId()) {
        setToolTipPosition();
        m_toolTip->updateContent();
        m_toolTip->setVisible(true);
        m_toolTipAppearTimer.stop();
    }

    if(event->timerId() == m_toolTipDisappearTimer.timerId()) {
        m_toolTip->setVisible(false);
        m_toolTipDisappearTimer.stop();
    }

    if(event->timerId() == m_filterApplianceTimer.timerId()) {
        m_modelFilterItems->setFilter(qVariantValue<KCategorizedItemsViewModels::Filter>
                                      (m_dataFilterAboutToApply));
        m_filterApplianceTimer.stop();
    }

    QGraphicsWidget::timerEvent(event);
}

void AppletsList::appletIconHoverEnter(AppletIconWidget *applet)
{
    if(!m_toolTip->isVisible()) {
        m_toolTip->setAppletIconWidget(applet);
        m_toolTipAppearTimer.start(TOOLTIP_APPEAR_DELAY, this);
    } else {
        if(!(m_toolTip->appletIconWidget()->appletItem()->pluginName() ==
            applet->appletItem()->pluginName())) {
            m_toolTip->setAppletIconWidget(applet);
            m_toolTip->updateContent();
            setToolTipPosition();
        }
        m_toolTipDisappearTimer.stop();
    }
}

void AppletsList::appletIconHoverLeave(AppletIconWidget *applet)
{
    Q_UNUSED(applet)

    if(m_toolTip->isVisible()) {
        m_toolTipDisappearTimer.start(TOOLTIP_DISAPPEAR_DELAY, this);
    } else {
        m_toolTipAppearTimer.stop();
    }
}

void AppletsList::onToolTipEnter()
{
    m_toolTipDisappearTimer.stop();
}

void AppletsList::onToolTipLeave()
{
    m_toolTipDisappearTimer.start(TOOLTIP_DISAPPEAR_DELAY, this);
}

void AppletsList::setToolTipPosition()
{
    QPointF appletPosition = m_toolTip->appletIconWidget()->mapToItem(this, 0, 0);

    Plasma::Corona *corona = dynamic_cast<Plasma::WidgetExplorer*>(parentItem())->corona();

    if(corona) {
        m_toolTip->move(corona->popupPosition(m_toolTip->appletIconWidget(), m_toolTip->contentsRect().size()));
    } else {
        m_toolTip->move(appletPosition.x(), appletPosition.y());
    }
}

void AppletsList::insertAppletIcon(AppletIconWidget *appletIconWidget)
{
    appletIconWidget->setVisible(true);
    m_appletListLinearLayout->addItem(appletIconWidget);
}

int AppletsList::maximumAproxVisibleIconsOnList() {
    qreal windowWidth = m_appletsListWindowWidget->geometry().width();
    qreal iconAverageSize = m_appletListLinearLayout->preferredSize().width()/
                          (m_currentAppearingAppletsOnList->count()
                           + m_appletListLinearLayout->spacing());
    //approximatelly
    qreal maxVisibleIconsOnList = floor(windowWidth/iconAverageSize);
    return maxVisibleIconsOnList;
}

AppletIconWidget *AppletsList::createAppletIcon(PlasmaAppletItem *appletItem)
{
    AppletIconWidget *applet = new AppletIconWidget(0, appletItem);
    applet->setMinimumSize(applet->sizeFromIconSize(70));
    applet->setMaximumSize(applet->sizeFromIconSize(70));

    connect(applet, SIGNAL(hoverEnter(AppletIconWidget*)), this, SLOT(appletIconHoverEnter(AppletIconWidget*)));
    connect(applet, SIGNAL(hoverLeave(AppletIconWidget*)), this, SLOT(appletIconHoverLeave(AppletIconWidget*)));
    connect(applet, SIGNAL(selected(AppletIconWidget*)), this, SLOT(itemSelected(AppletIconWidget*)));
    connect(applet, SIGNAL(doubleClicked(AppletIconWidget*)), this, SLOT(appletIconDoubleClicked(AppletIconWidget*)));

    return applet;
}

void AppletsList::itemSelected(AppletIconWidget *applet)
{
    if(m_selectedItem) {
        m_selectedItem->setSelected(false);
    }
    applet->setSelected(true);
    m_selectedItem = applet;
}

void AppletsList::appletIconDoubleClicked(AppletIconWidget *applet)
{
    emit(appletDoubleClicked(applet->appletItem()));
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
    m_appletListLinearLayout->setContentsMargins(0, 0, 0, 0);

    eraseList();

    for(int i = 0; i < m_modelFilterItems->rowCount(); i++) {
        item = getItemByProxyIndex(m_modelFilterItems->index(i, 0));
        appletItem = (PlasmaAppletItem*) item;

        if(appletItem != 0) {
            appletIconWidget = m_allAppletsHash->value(appletItem->id());
            insertAppletIcon(appletIconWidget);
            m_currentAppearingAppletsOnList->append(appletIconWidget);
        }
    }

    m_appletsListWidget->setLayout(m_appletListLinearLayout);
    m_appletListLinearLayout->setSpacing(10);
    resetScroll();
}

void AppletsList::wheelEvent(QGraphicsSceneWheelEvent *event) {
    //if the scroll is right or left
    bool right;
    bool byWheel = true;
    bool canScroll;

    right = event->delta() < 0 ? true : false;

    if(right) {
        canScroll = m_rightArrow->isEnabled() ? true : false;
    } else {
        canScroll = m_leftArrow->isEnabled() ? true : false;
    }

    if(canScroll) {
        scroll(right, byWheel);
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

    m_toolTip->setVisible(false);

    if(right) {
        scrollRight(step, visibleRect);
    } else {
        scrollLeft(step, visibleRect);
    }
}

void AppletsList::scrollRight(int step, QRectF visibleRect) {
    qreal scrollXAmount;
    int lastVisibleXOnList = visibleRect.x() + visibleRect.width();
    int lastVisibleItemIndex = findLastVisibleApplet(lastVisibleXOnList);
    AppletIconWidget *newLastIcon;
    int appletsIndexesToSum;

    qDebug() << "step " << step;
    qDebug() << "lastVisibleItemIndex " << lastVisibleItemIndex;

    if(isItemUnder(lastVisibleItemIndex, lastVisibleXOnList)) {
        appletsIndexesToSum = step - 1;
    } else {
        appletsIndexesToSum = step;
    }

    if(lastVisibleItemIndex + appletsIndexesToSum >= m_currentAppearingAppletsOnList->count()) {
        appletsIndexesToSum = m_currentAppearingAppletsOnList->count() - 1 - lastVisibleItemIndex;
    }

    newLastIcon = m_currentAppearingAppletsOnList->at(lastVisibleItemIndex + appletsIndexesToSum);
    scrollXAmount = (newLastIcon->pos().x() + newLastIcon->size().width()) -
                        lastVisibleXOnList;

    //check if the scrollAmount is more than necessary to reach the end of the list
    qreal listWidth = m_appletListLinearLayout->preferredSize().width();
    if(lastVisibleXOnList + scrollXAmount > listWidth) {
        scrollXAmount = listWidth - lastVisibleXOnList;
    }

    animateMoveBy(-scrollXAmount);
    emit(listScrolled());
}

void AppletsList::scrollLeft(int step, QRectF visibleRect) {
    qreal scrollXAmount;
    int firstVisibleXOnList = visibleRect.x();
    int firstVisibleItemIndex = findFirstVisibleApplet(firstVisibleXOnList);
    AppletIconWidget *newFirstIcon;
    int appletsIndexesToReduce;

    qDebug() << "step " << step;
    qDebug() << "firstVisibleItemIndex " << firstVisibleItemIndex;

    if(isItemUnder(firstVisibleItemIndex, firstVisibleXOnList)) {
        qDebug() << "is under";
        appletsIndexesToReduce = step - 1;
    } else {
        appletsIndexesToReduce = step;
    }

    if(firstVisibleItemIndex - appletsIndexesToReduce < 0) {
        appletsIndexesToReduce = firstVisibleItemIndex;
    }

    newFirstIcon = m_currentAppearingAppletsOnList->at(firstVisibleItemIndex - appletsIndexesToReduce);
    scrollXAmount = firstVisibleXOnList - newFirstIcon->pos().x();

    //check if the scrollAmount is more than necessary to reach the begining of the list
    if(firstVisibleXOnList - scrollXAmount <  0){
        scrollXAmount = firstVisibleXOnList;
    }

    animateMoveBy(scrollXAmount);
    emit(listScrolled());
}

/* ivan: Implementation of the following functions is
 * less than optimal and is intended just to provide
 * the temporary solution until plasma gets the new
 * animation framework.
 * TODO: Remove this and animate using plasma's
 * animation framework when it is created */
void AppletsList::animateMoveBy(int amount)
{
    scrollFrom = m_appletsListWidget->pos().x();
    if (scrollTimeLine.state() == QTimeLine::Running) {
        scrollTo = scrollTo + amount;
    } else {
        scrollTo = scrollFrom + amount;
    }

    if (scrollTo > 0) {
        scrollTo = 0;
    }

    if (scrollTo + m_appletsListWidget->size().width() <
        m_appletsListWindowWidget->size().width()) {
        scrollTo = m_appletsListWindowWidget->size().width()
                   - m_appletsListWidget->size().width();
    }

    // scrollTimeLine.stop();

    if (scrollTimeLine.state() != QTimeLine::Running &&
            scrollFrom != scrollTo) {
        scrollTimeLine.start();
    }
}

void AppletsList::scrollTimeLineFrameChanged(int frame)
{
    QPointF newPos = QPointF(
        (frame/(qreal)100) * (scrollTo - scrollFrom) + scrollFrom,
        m_appletsListWidget->pos().y());
    m_appletsListWidget->setPos(newPos);

    if (frame == 100) {
        manageArrows();
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
    qreal listWidthVar = m_appletListLinearLayout->preferredSize().width();

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

bool AppletsList::isItemUnder(int itemIndex, qreal xPosition) {
    int firstXOnApplet;
    int lastXOnApplet;
    AppletIconWidget *applet;

    applet = m_currentAppearingAppletsOnList->at(itemIndex);
    qDebug() << applet->appletItem()->name();
    firstXOnApplet = applet->mapToItem(m_appletsListWidget, 0, 0).x();
    lastXOnApplet = applet->mapToItem(m_appletsListWidget, applet->boundingRect().width(), 0).x();
    if((xPosition > firstXOnApplet) && (xPosition < lastXOnApplet)) {
        qDebug() << applet->appletItem()->name();
        return true;
    } else {
        return false;
    }
}

int AppletsList::findFirstVisibleApplet(int firstVisibleXOnList) {

    int resultIndex = -1;
    int resultDistance = 99999;
    int tempDistance = 0;
    AppletIconWidget *applet;

    for(int i = 0; i < m_currentAppearingAppletsOnList->count(); i++) {
        applet = m_currentAppearingAppletsOnList->at(i);
        tempDistance = applet->pos().x() + applet->size().width() - firstVisibleXOnList;
        if (tempDistance <= resultDistance && tempDistance > 0) {
            resultIndex = i;
            resultDistance = tempDistance;
        }
    }
    return resultIndex;
}

int AppletsList::findLastVisibleApplet(int lastVisibleXOnList) {

    int resultIndex = -1;
    int resultDistance = 99999;
    int tempDistance = 0;

    for(int i = 0; i < m_currentAppearingAppletsOnList->count(); i++) {
        tempDistance = lastVisibleXOnList - m_currentAppearingAppletsOnList->at(i)->pos().x();
        if (tempDistance <= resultDistance && tempDistance > 0) {
            resultIndex = i;
            resultDistance = tempDistance;
        }
    }

    return resultIndex;
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

QList <AbstractItem *> AppletsList::selectedItems() const
{
//    return m_appletList->selectedItems();
    return QList<AbstractItem *>();
}
