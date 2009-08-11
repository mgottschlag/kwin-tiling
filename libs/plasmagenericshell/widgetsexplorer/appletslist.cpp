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

AppletsList::AppletsList(Qt::Orientation orientation, QGraphicsItem *parent)
        :QGraphicsWidget(parent)
{
    arrowClickStep = 0;
    wheelStep = 0;
    m_selectedItem = 0;
    m_currentAppearingAppletsOnList = new QList<AppletIconWidget *>();
    m_orientation = orientation;

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

    init();
}

AppletsList::~AppletsList()
{
}

void AppletsList::init()
{
    m_upLeftArrow = new Plasma::PushButton();
    m_upLeftArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));

    m_downRightArrow = new Plasma::PushButton();
    m_downRightArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));

    connect(m_downRightArrow, SIGNAL(clicked()), this, SLOT(onRightArrowClick()));
    connect(m_upLeftArrow, SIGNAL(clicked()), this, SLOT(onLeftArrowClick()));

    m_appletsListWidget = new QGraphicsWidget();
    m_appletListLinearLayout = new QGraphicsLinearLayout(m_orientation);
    m_appletsListWidget->setLayout(m_appletListLinearLayout);
    m_appletsListWidget->installEventFilter(this);

    m_appletsListWindowWidget = new QGraphicsWidget();
    m_appletsListWindowWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    m_appletsListWidget->setParentItem(m_appletsListWindowWidget);
    m_appletsListWindowWidget->installEventFilter(this);

    m_arrowsLayout = new QGraphicsLinearLayout(m_orientation);

    m_arrowsLayout->addItem(m_upLeftArrow);
    m_arrowsLayout->addItem(m_appletsListWindowWidget);
    m_arrowsLayout->addItem(m_downRightArrow);

    m_arrowsLayout->setAlignment(m_downRightArrow, Qt::AlignVCenter | Qt::AlignHCenter);
    m_arrowsLayout->setAlignment(m_upLeftArrow, Qt::AlignVCenter | Qt::AlignHCenter);
    m_arrowsLayout->setAlignment(m_appletsListWindowWidget, Qt::AlignVCenter | Qt::AlignHCenter);

    if(m_orientation == Qt::Horizontal) {
        m_upLeftArrow->nativeWidget()->setIcon(KIcon("go-previous"));
        m_downRightArrow->nativeWidget()->setIcon(KIcon("go-next"));
    } else {
        m_upLeftArrow->nativeWidget()->setIcon(KIcon("go-up"));
        m_downRightArrow->nativeWidget()->setIcon(KIcon("go-down"));
    }

    setLayout(m_arrowsLayout);
}

void AppletsList::resizeEvent(QGraphicsSceneResizeEvent *event) {

    Q_UNUSED(event);

    updateGeometry();

    if(m_orientation == Qt::Horizontal) {
        m_appletsListWindowWidget->setMinimumWidth(-1);
        m_appletsListWindowWidget->setMaximumWidth(-1);
    } else {
        m_appletsListWindowWidget->setMinimumHeight(-1);
        m_appletsListWindowWidget->setMaximumHeight(-1);
    }
    manageArrows();
}

bool AppletsList::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsSceneResizeEvent *resizeEvent = static_cast<QGraphicsSceneResizeEvent *>(event);
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(obj);

        if(widget == m_appletsListWidget) {
            if(m_orientation == Qt::Horizontal) {
                m_appletsListWindowWidget->setMinimumHeight(resizeEvent->newSize().height());
                m_appletsListWindowWidget->setMaximumHeight(resizeEvent->newSize().height());
            } else {
                m_appletsListWindowWidget->setMinimumWidth(resizeEvent->newSize().width());
                m_appletsListWindowWidget->setMaximumWidth(resizeEvent->newSize().width());
            }
            return false;
        } else if(widget == m_appletsListWindowWidget) {
            int maxVisibleIconsOnList = maximumAproxVisibleIconsOnList();
            arrowClickStep = ceil(maxVisibleIconsOnList/4);
            wheelStep = ceil(maxVisibleIconsOnList/2);
            return false;
        }

     }

    return QObject::eventFilter(obj, event);
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

void AppletsList::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    adjustContentsAccordingToOrientation();
}

void AppletsList::adjustContentsAccordingToOrientation()
{
    m_appletListLinearLayout->invalidate();
    m_appletListLinearLayout->setOrientation(m_orientation);
    m_arrowsLayout->setOrientation(m_orientation);

    if(m_orientation == Qt::Horizontal) {
        m_upLeftArrow->nativeWidget()->setIcon(KIcon("go-previous"));
        m_downRightArrow->nativeWidget()->setIcon(KIcon("go-next"));
    } else {
        m_upLeftArrow->nativeWidget()->setIcon(KIcon("go-up"));
        m_downRightArrow->nativeWidget()->setIcon(KIcon("go-down"));
    }

    m_appletListLinearLayout->activate();
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
        m_toolTip->move(corona->popupPosition(m_toolTip->appletIconWidget(), m_toolTip->geometry().size()));
    } else {
        m_toolTip->move(appletPosition.x(), appletPosition.y());
    }
}

void AppletsList::insertAppletIcon(AppletIconWidget *appletIconWidget)
{
    appletIconWidget->setVisible(true);
    m_appletListLinearLayout->addItem(appletIconWidget);
    m_appletListLinearLayout->setAlignment(appletIconWidget, Qt::AlignHCenter);
}

int AppletsList::maximumAproxVisibleIconsOnList() {
    qreal windowSize;
    qreal listTotalSize;
    qreal iconAverageSize;
    qreal maxVisibleIconsOnList;

    if(m_orientation == Qt::Horizontal) {
        windowSize = m_appletsListWindowWidget->geometry().width();
        listTotalSize = m_appletListLinearLayout->preferredSize().width();
    } else {
        windowSize = m_appletsListWindowWidget->geometry().height();
        listTotalSize = m_appletListLinearLayout->preferredSize().height();
    }

    iconAverageSize = listTotalSize/
                      (m_currentAppearingAppletsOnList->count()) +
                       m_appletListLinearLayout->spacing();
//    approximatelly
    maxVisibleIconsOnList = floor(windowSize/iconAverageSize);

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
    m_appletListLinearLayout = new QGraphicsLinearLayout(m_orientation);
    m_currentAppearingAppletsOnList->clear();

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
    ScrollPolicy side;
    bool canScroll;

    side = event->delta() < 0 ? AppletsList::DownRight : AppletsList::UpLeft;

    if(side == AppletsList::DownRight) {
        canScroll = m_downRightArrow->isEnabled() ? true : false;
    } else {
        canScroll = m_upLeftArrow->isEnabled() ? true : false;
    }

    if(canScroll) {
        scroll(side, AppletsList::Wheel);
    }
}

void AppletsList::onRightArrowClick() {
    scroll(AppletsList::DownRight, AppletsList::Button);
}

void AppletsList::onLeftArrowClick() {
    scroll(AppletsList::UpLeft, AppletsList::Button);
}

void AppletsList::scroll(ScrollPolicy side, ScrollPolicy how) {
    QRectF visibleRect = visibleListRect();
    int step = (how == AppletsList::Wheel) ? wheelStep : arrowClickStep;

    m_toolTip->setVisible(false);

    switch(side) {
        case AppletsList::DownRight:
            scrollDownRight(step, visibleRect);
            break;
        case AppletsList::UpLeft:
            scrollUpLeft(step, visibleRect);
            break;
        default:
            break;
    }
}

void AppletsList::scrollDownRight(int step, QRectF visibleRect) {
    qreal scrollAmount;
    int lastVisiblePositionOnList;
    int lastVisibleItemIndex;
    AppletIconWidget *newLastIcon;
    int appletsIndexesToSum;
    qreal listSize;

    if(m_orientation == Qt::Horizontal) {
        lastVisiblePositionOnList = visibleRect.x() + visibleRect.width();
    } else {
        lastVisiblePositionOnList = visibleRect.y() + visibleRect.height();
    }

    lastVisibleItemIndex = findLastVisibleApplet(lastVisiblePositionOnList);

    qDebug() << "step " << step;
    qDebug() << "lastVisibleItemIndex " << lastVisibleItemIndex;

    //find out if the last icon on list appears clipped
    if(isItemUnder(lastVisibleItemIndex, lastVisiblePositionOnList)) {
        qDebug() << "clipped";
        appletsIndexesToSum = step - 1;
    } else {
        qDebug() << "not clipped";
        appletsIndexesToSum = step;
    }

    if(lastVisibleItemIndex + appletsIndexesToSum >= m_currentAppearingAppletsOnList->count()) {
        appletsIndexesToSum = m_currentAppearingAppletsOnList->count() - 1 - lastVisibleItemIndex;
    }

    newLastIcon = m_currentAppearingAppletsOnList->at(lastVisibleItemIndex + appletsIndexesToSum);

    if(m_orientation == Qt::Horizontal) {
        scrollAmount = (newLastIcon->pos().x() + newLastIcon->size().width()) -
                        lastVisiblePositionOnList;
        listSize = m_appletListLinearLayout->preferredSize().width();
    } else {
        scrollAmount = (newLastIcon->pos().y() + newLastIcon->size().height()) -
                        lastVisiblePositionOnList;
        listSize = m_appletListLinearLayout->preferredSize().height();
    }

    //check if the scrollAmount is more than necessary to reach the end of the list
    if(lastVisiblePositionOnList + scrollAmount > listSize) {
        scrollAmount = listSize - lastVisiblePositionOnList;
    }

    animateMoveBy(-scrollAmount);
    emit(listScrolled());
}

void AppletsList::scrollUpLeft(int step, QRectF visibleRect) {
    qreal scrollAmount;
    int firstVisiblePositionOnList;
    int firstVisibleItemIndex;
    AppletIconWidget *newFirstIcon;
    int appletsIndexesToReduce;

    if(m_orientation == Qt::Horizontal) {
        firstVisiblePositionOnList = visibleRect.x();
    } else {
        firstVisiblePositionOnList = visibleRect.y();
    }

    firstVisibleItemIndex = findFirstVisibleApplet(firstVisiblePositionOnList);

    qDebug() << "step " << step;
    qDebug() << "firstVisibleItemIndex " << firstVisibleItemIndex;

    //find out if the last icon on list appears clipped
    if(isItemUnder(firstVisibleItemIndex, firstVisiblePositionOnList)) {
        qDebug() << "is under";
        appletsIndexesToReduce = step - 1;
    } else {
        qDebug() << "not under";
        appletsIndexesToReduce = step;
    }

    if(firstVisibleItemIndex - appletsIndexesToReduce < 0) {
        appletsIndexesToReduce = firstVisibleItemIndex;
    }

    newFirstIcon = m_currentAppearingAppletsOnList->at(firstVisibleItemIndex - appletsIndexesToReduce);

    if(m_orientation == Qt::Horizontal) {
        scrollAmount = firstVisiblePositionOnList - newFirstIcon->pos().x();
    } else {
        scrollAmount = firstVisiblePositionOnList - newFirstIcon->pos().y();
    }

    //check if the scrollAmount is more than necessary to reach the begining of the list
    if(firstVisiblePositionOnList - scrollAmount <  0){
        scrollAmount = firstVisiblePositionOnList;
    }

    animateMoveBy(scrollAmount);
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
    if(m_orientation == Qt::Horizontal) {
        scrollFrom = m_appletsListWidget->pos().x();
    } else {
        scrollFrom = m_appletsListWidget->pos().y();
    }

    if (scrollTimeLine.state() == QTimeLine::Running) {
        scrollTo = scrollTo + amount;
    } else {
        scrollTo = scrollFrom + amount;
    }

    if (scrollTo > 0) {
        scrollTo = 0;
    }

    if(m_orientation == Qt::Horizontal) {
        if (scrollTo + m_appletsListWidget->size().width() <
            m_appletsListWindowWidget->size().width()) {
            scrollTo = m_appletsListWindowWidget->size().width()
                       - m_appletsListWidget->size().width();
        }
    } else {
        if (scrollTo + m_appletsListWidget->size().height() <
            m_appletsListWindowWidget->size().height()) {
            scrollTo = m_appletsListWindowWidget->size().height()
                       - m_appletsListWidget->size().height();
        }
    }

    // scrollTimeLine.stop();

    if (scrollTimeLine.state() != QTimeLine::Running &&
            scrollFrom != scrollTo) {
        scrollTimeLine.start();
    }
}

void AppletsList::scrollTimeLineFrameChanged(int frame)
{
    QPointF newPos;

    if(m_orientation == Qt::Horizontal) {
        newPos = QPointF(
                (frame/(qreal)100) * (scrollTo - scrollFrom) + scrollFrom,
                m_appletsListWidget->pos().y());

    } else {
        newPos = QPointF(
                m_appletsListWidget->pos().x(),
                (frame/(qreal)100) * (scrollTo - scrollFrom) + scrollFrom);
    }

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
    QRectF visibleRect = visibleListRect();
    qreal firstVisiblePositionOnList;
    qreal lastVisiblePositionOnList;
    qreal listSize;
    qreal windowSize;

    if(m_orientation == Qt::Horizontal) {
        windowSize = m_appletsListWindowWidget->geometry().width();
        listSize = m_appletListLinearLayout->preferredSize().width();
    } else {
        windowSize = m_appletsListWindowWidget->geometry().height();
        listSize = m_appletListLinearLayout->preferredSize().height();
    }

    if(listSize <= windowSize) {
        m_upLeftArrow->setEnabled(false);
        m_downRightArrow->setEnabled(false);
    } else {

        if(m_orientation == Qt::Horizontal) {
            windowSize = m_appletsListWindowWidget->geometry().width();
            firstVisiblePositionOnList = visibleRect.x();
            lastVisiblePositionOnList = firstVisiblePositionOnList + visibleRect.width();
        } else {
            windowSize = m_appletsListWindowWidget->geometry().height();
            firstVisiblePositionOnList = visibleRect.y();
            lastVisiblePositionOnList = firstVisiblePositionOnList + visibleListRect().height();
        }

        if(firstVisiblePositionOnList <= m_appletListLinearLayout->spacing()) {
            m_upLeftArrow->setEnabled(false);
        } else {
            m_upLeftArrow->setEnabled(true);
        }

        if(lastVisiblePositionOnList >= listSize - m_appletListLinearLayout->spacing()) {
            m_downRightArrow->setEnabled(false);
        } else {
            m_downRightArrow->setEnabled(true);
        }
    }
}

bool AppletsList::isItemUnder(int itemIndex, qreal position) {
    int firstPositionOnApplet;
    int lastPositionOnApplet;
    AppletIconWidget *applet;

    applet = m_currentAppearingAppletsOnList->at(itemIndex);
    
    if(m_orientation == Qt::Horizontal) {
        firstPositionOnApplet = applet->mapToItem(m_appletsListWidget, 0, 0).x();
        lastPositionOnApplet = applet->mapToItem
                               (m_appletsListWidget, applet->boundingRect().width(), 0).x();
    } else {
        firstPositionOnApplet = applet->mapToItem(m_appletsListWidget, 0, 0).y();
        lastPositionOnApplet = applet->mapToItem
                               (m_appletsListWidget, 0, applet->boundingRect().height()).y();
    }

    if((position > firstPositionOnApplet) && (position < lastPositionOnApplet)) {
        return true;
    } else {
        return false;
    }
}

int AppletsList::findFirstVisibleApplet(int firstVisiblePositionOnList) {

    int resultIndex = -1;
    int resultDistance = 99999;
    int tempDistance = 0;
    AppletIconWidget *applet;

    for(int i = 0; i < m_currentAppearingAppletsOnList->count(); i++) {
        applet = m_currentAppearingAppletsOnList->at(i);

        if(m_orientation == Qt::Horizontal) {
            tempDistance = applet->pos().x() + applet->size().width() - firstVisiblePositionOnList;
        } else {
            tempDistance = applet->pos().y() + applet->size().height() - firstVisiblePositionOnList;
        }
        if (tempDistance <= resultDistance && tempDistance > 0) {
            resultIndex = i;
            resultDistance = tempDistance;
        }
    }
    return resultIndex;
}

int AppletsList::findLastVisibleApplet(int lastVisiblePositionOnList) {

    int resultIndex = -1;
    int resultDistance = 99999;
    int tempDistance = 0;

    for(int i = 0; i < m_currentAppearingAppletsOnList->count(); i++) {
        if(m_orientation == Qt::Horizontal) {
            tempDistance = lastVisiblePositionOnList -
                           m_currentAppearingAppletsOnList->at(i)->pos().x();
        } else {
            tempDistance = lastVisiblePositionOnList -
                           m_currentAppearingAppletsOnList->at(i)->pos().y();
        }

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
