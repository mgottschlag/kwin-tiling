/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2009 by Ivan Cukic <ivan.cukic+kde@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library/Lesser General Public License
 *   version 2, or (at your option) any later version, as published by the
 *   Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library/Lesser General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include "appletslist.h"

#include <cmath>

#include <QHash>

#include <KIconLoader>
#include <KIcon>
#include <KPushButton>

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/ItemBackground>
#include <Plasma/Theme>

#include "widgetexplorer.h"

const int ICON_SIZE = 70;
const int FILTER_APPLIANCE_DELAY = 400;
const int SEARCH_DELAY = 300;
const int TOOLTIP_APPEAR_DELAY = 1000;
const int TOOLTIP_APPEAR_WHEN_VISIBLE_DELAY = 300;
const int TOOLTIP_DISAPPEAR_DELAY = 300;
const int SCROLL_STEP_DURATION = 300;

using namespace KCategorizedItemsViewModels;

AppletsListWidget::AppletsListWidget(Qt::Orientation orientation, QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_selectionIndicator(new Plasma::ItemBackground(this)),
      m_hoverIndicator(new Plasma::ItemBackground(this))
{
    arrowClickStep = 0;
    wheelStep = 0;
    m_selectedItem = 0;
    m_currentAppearingAppletsOnList = new QList<AppletIconWidget *>();
    m_orientation = orientation;

    //init arrows svg
    m_arrowsSvg = new Plasma::Svg(this);
    m_arrowsSvg->setImagePath("widgets/arrows");
    m_arrowsSvg->setContainsMultipleImages(true);
    m_arrowsSvg->resize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);

    connect(this, SIGNAL(listScrolled()), this, SLOT(manageArrows()));

    //init timelines
    m_scrollTimeLine.setFrameRange(0, 100);
    //m_scrollTimeLine.setCurveShape(QTimeLine::EaseInOutCurve);
    m_scrollTimeLine.setCurveShape(QTimeLine::LinearCurve);
    m_scrollTimeLine.setDuration(SCROLL_STEP_DURATION); // TODO: Set this to a lesser value
    connect(&m_scrollTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(scrollTimeLineFrameChanged(int)));

    toolTipMoveTimeLine.setFrameRange(0, 100);
    toolTipMoveTimeLine.setCurveShape(QTimeLine::EaseInOutCurve);
    toolTipMoveTimeLine.setDuration(500);
    connect(&toolTipMoveTimeLine, SIGNAL(frameChanged(int)),
            this, SLOT(toolTipMoveTimeLineFrameChanged(int)));

    //init tooltip
    m_toolTip = new AppletToolTipWidget();
    m_toolTip->setVisible(false);
    connect(m_toolTip, SIGNAL(enter()), this, SLOT(onToolTipEnter()));
    connect(m_toolTip, SIGNAL(leave()), this, SLOT(onToolTipLeave()));

    init();
}

AppletsListWidget::~AppletsListWidget()
{
    delete m_toolTip;
}

void AppletsListWidget::init()
{
    //init arrows
    m_upLeftArrow = new Plasma::PushButton();
    m_upLeftArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));

    m_downRightArrow = new Plasma::PushButton();
    m_downRightArrow->setMaximumSize(IconSize(KIconLoader::Panel), IconSize(KIconLoader::Panel));

    if (m_orientation == Qt::Horizontal) {
        m_upLeftArrow->setIcon(KIcon(m_arrowsSvg->pixmap("left-arrow")));
        m_downRightArrow->setIcon(KIcon(m_arrowsSvg->pixmap("right-arrow")));
        m_upLeftArrow->setMaximumSize(IconSize(KIconLoader::Panel), -1);
        m_downRightArrow->setMaximumSize(IconSize(KIconLoader::Panel), -1);
    } else {
        m_upLeftArrow->setIcon(KIcon(m_arrowsSvg->pixmap("up-arrow")));
        m_downRightArrow->setIcon(KIcon(m_arrowsSvg->pixmap("down-arrow")));
        m_upLeftArrow->setMaximumSize(-1, IconSize(KIconLoader::Panel));
        m_downRightArrow->setMaximumSize(-1, IconSize(KIconLoader::Panel));
    }

    connect(m_downRightArrow, SIGNAL(pressed()), this, SLOT(onRightArrowPress()));
    connect(m_upLeftArrow, SIGNAL(pressed()), this, SLOT(onLeftArrowPress()));

    //init applets list
    m_appletsListWidget = new QGraphicsWidget();
    m_appletListLinearLayout = new QGraphicsLinearLayout(m_orientation);
    m_appletsListWidget->setLayout(m_appletListLinearLayout);
    //make its events pass through its parent
    m_appletsListWidget->installEventFilter(this);

    //init window that shows the applets of the list - it clips the appletsListWidget
    m_appletsListWindowWidget = new QGraphicsWidget();
    m_appletsListWindowWidget->setFlag(QGraphicsItem::ItemClipsChildrenToShape, true);
    m_appletsListWidget->setParentItem(m_appletsListWindowWidget);
    //make its events pass through its parent
    m_appletsListWindowWidget->installEventFilter(this);

    //layouts
    m_arrowsLayout = new QGraphicsLinearLayout(m_orientation);

    m_arrowsLayout->addItem(m_upLeftArrow);
    m_arrowsLayout->addItem(m_appletsListWindowWidget);
    m_arrowsLayout->addItem(m_downRightArrow);

    m_arrowsLayout->setAlignment(m_downRightArrow, Qt::AlignVCenter | Qt::AlignHCenter);
    m_arrowsLayout->setAlignment(m_upLeftArrow, Qt::AlignVCenter | Qt::AlignHCenter);
    m_arrowsLayout->setAlignment(m_appletsListWindowWidget, Qt::AlignVCenter | Qt::AlignHCenter);

    setLayout(m_arrowsLayout);

    themeUpdated();
    connect(Plasma::Theme::defaultTheme(), SIGNAL(themeChanged()), this, SLOT(themeUpdated()));
}

void AppletsListWidget::themeUpdated()
{
    Plasma::Theme *theme = Plasma::Theme::defaultTheme();
    QColor buttonBgColor = theme->color(Plasma::Theme::BackgroundColor);
    QString buttonStyleSheet = QString("Plasma::PushButton { border: 1px solid %4; border-radius: 4px; padding: 2px;"
                                       " background-color: rgba(%1, %2, %3, %5); }")
                                      .arg(buttonBgColor.red())
                                      .arg(buttonBgColor.green())
                                      .arg(buttonBgColor.blue())
                                      .arg(theme->color(Plasma::Theme::BackgroundColor).name(), "50%");
    buttonBgColor = theme->color(Plasma::Theme::TextColor);
    buttonStyleSheet += QString("Plasma::PushButton:hover { border: 2px solid %1; }")
                               .arg(theme->color(Plasma::Theme::HighlightColor).name());
    buttonStyleSheet += QString("Plasma::PushButton:focus { border: 2px solid %1; }")
                               .arg(theme->color(Plasma::Theme::HighlightColor).name());

    m_upLeftArrow->nativeWidget()->setStyleSheet(buttonStyleSheet);
    m_downRightArrow->nativeWidget()->setStyleSheet(buttonStyleSheet);
}

void AppletsListWidget::resizeEvent(QGraphicsSceneResizeEvent *event)
{
    Q_UNUSED(event);

    if (m_orientation == Qt::Horizontal) {
        m_appletsListWindowWidget->setMinimumWidth(-1);
        m_appletsListWindowWidget->setMaximumWidth(-1);
    } else {
        m_appletsListWindowWidget->setMinimumHeight(-1);
        m_appletsListWindowWidget->setMaximumHeight(-1);
    }

    kDebug() << "m_appletsListWidget->size()" << m_appletsListWidget->size();
    kDebug() << "size()" << size();

    qreal contentMarginTop;
    qreal contentMarginBottom;
    qreal contentMarginLeft;
    qreal contentMarginRight;
    int height = event->newSize().height();
    int width = event->newSize().width();
    int iconSize;

    if (m_orientation == Qt::Horizontal) {
        m_arrowsLayout->getContentsMargins(&contentMarginLeft, &contentMarginTop,
                                           &contentMarginRight, &contentMarginBottom);
        height -= (contentMarginBottom + contentMarginTop);
        kDebug() << "resize: old this size" << event->oldSize().height();
        kDebug() << "resize: new this size" << event->newSize().height();
        kDebug() << "resize: height" << height;
        m_appletsListWidget->resize(m_appletsListWidget->size().width(), height);
        iconSize = height;
        m_appletListLinearLayout->getContentsMargins(&contentMarginLeft, &contentMarginTop,
                                                     &contentMarginRight, &contentMarginBottom);
        iconSize -= (contentMarginBottom + contentMarginTop);
    } else {
        //i don't know why, but when it's vertical, a loop happens here:
        //m_appletsListWidget resizes and it causes a resize to this
        m_arrowsLayout->getContentsMargins(&contentMarginLeft, &contentMarginTop,
                                       &contentMarginRight, &contentMarginBottom);
        width -= (contentMarginLeft + contentMarginRight);
        kDebug() << "resize: old this size" << event->oldSize().width();
        kDebug() << "resize: new this size" << event->newSize().width();
        kDebug() << "resize: width" << width;
        m_appletsListWidget->resize(width, m_appletsListWidget->size().height());
        iconSize = width;
        m_appletListLinearLayout->getContentsMargins(&contentMarginLeft, &contentMarginTop,
                                                     &contentMarginRight, &contentMarginBottom);
        iconSize -= (contentMarginLeft + contentMarginRight);
    }

    foreach (AppletIconWidget *applet, m_allAppletsHash) { 
        applet->setMinimumSize(iconSize, iconSize);
        applet->setMaximumSize(iconSize, iconSize);
    }
}

//parent intercepts children events
bool AppletsListWidget::eventFilter(QObject *obj, QEvent *event)
{
    if (event->type() == QEvent::GraphicsSceneResize) {
        QGraphicsWidget *widget = dynamic_cast<QGraphicsWidget *>(obj);

        if (widget == m_appletsListWidget) {
            //the resize occured with the list widget
            manageArrows();
            return false;
        } else if (widget == m_appletsListWindowWidget) {
            // the resize occured with the window widget
            int maxVisibleIconsOnList = maximumAproxVisibleIconsOnList();
            arrowClickStep = ceil((float)maxVisibleIconsOnList/4);
            wheelStep = ceil((float)maxVisibleIconsOnList/2);
            return false;
        }
    }

    return QObject::eventFilter(obj, event);
}

void AppletsListWidget::setItemModel(PlasmaAppletItemModel *model)
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

void AppletsListWidget::setFilterModel(QStandardItemModel *model)
{
    m_modelFilters = model;
}

void AppletsListWidget::setOrientation(Qt::Orientation orientation)
{
    m_orientation = orientation;
    setContentsPropertiesAccordingToOrientation();
    updateList();
}

void AppletsListWidget::setContentsPropertiesAccordingToOrientation()
{
    m_appletListLinearLayout->invalidate();
    m_appletListLinearLayout->setOrientation(m_orientation);
    m_arrowsLayout->setOrientation(m_orientation);

    if(m_orientation == Qt::Horizontal) {
        m_upLeftArrow->nativeWidget()->setIcon(KIcon(QIcon(m_arrowsSvg->pixmap("left-arrow"))));
        m_downRightArrow->nativeWidget()->setIcon(KIcon(QIcon(m_arrowsSvg->pixmap("right-arrow"))));
        m_upLeftArrow->setMaximumSize(IconSize(KIconLoader::Panel), -1);
        m_downRightArrow->setMaximumSize(IconSize(KIconLoader::Panel), -1);

    } else {
        m_upLeftArrow->nativeWidget()->setIcon(KIcon(QIcon(m_arrowsSvg->pixmap("up-arrow"))));
        m_downRightArrow->nativeWidget()->setIcon(KIcon(QIcon(m_arrowsSvg->pixmap("down-arrow"))));
        m_upLeftArrow->setMaximumSize(-1, IconSize(KIconLoader::Panel));
        m_downRightArrow->setMaximumSize(-1, IconSize(KIconLoader::Panel));
    }

    m_appletListLinearLayout->activate();
}

void AppletsListWidget::filterChanged(int index)
{
    if (m_modelFilterItems) {
        QStandardItem *item = m_modelFilters->item(index);

        if (item) {
            m_dataFilterAboutToApply = item->data();
            //wait a little before filtering the list
            m_filterApplianceTimer.start(FILTER_APPLIANCE_DELAY, this);
        }
    }
}

void AppletsListWidget::searchTermChanged(const QString &text)
{
    m_searchString = text;
    m_searchDelayTimer.start(SEARCH_DELAY, this);
}

void AppletsListWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_searchDelayTimer.timerId()) {
        m_modelFilterItems->setSearch(m_searchString);
        m_searchDelayTimer.stop();
    } else if (event->timerId() == m_toolTipAppearTimer.timerId()) {
        setToolTipPosition();
        m_toolTip->updateContent();
        m_toolTip->setVisible(true);
        m_toolTipAppearTimer.stop();
    } else if (event->timerId() == m_toolTipAppearWhenAlreadyVisibleTimer.timerId()) {
        m_toolTip->updateContent();
        setToolTipPosition();
        m_toolTipAppearWhenAlreadyVisibleTimer.stop();
    } else if (event->timerId() == m_toolTipDisappearTimer.timerId()) {
        m_toolTip->setVisible(false);
        m_toolTipDisappearTimer.stop();
    } else if (event->timerId() == m_filterApplianceTimer.timerId()) {
        m_modelFilterItems->setFilter(qVariantValue<KCategorizedItemsViewModels::Filter>
                                      (m_dataFilterAboutToApply));
        m_filterApplianceTimer.stop();
    }

    QGraphicsWidget::timerEvent(event);
}

void AppletsListWidget::appletIconHoverEnter(AppletIconWidget *applet)
{
    if (!m_toolTip->isVisible()) {
        m_toolTip->setAppletIconWidget(applet);
        m_toolTipAppearTimer.start(TOOLTIP_APPEAR_DELAY, this);
    } else {
        if(!(m_toolTip->appletIconWidget()->appletItem()->pluginName() ==
            applet->appletItem()->pluginName())) {
            m_toolTip->setAppletIconWidget(applet);

            //small delay, so if one's hovering very fast over the icons,
            //the tooltip doesn't appear frantically
            m_toolTipAppearWhenAlreadyVisibleTimer.start(TOOLTIP_APPEAR_WHEN_VISIBLE_DELAY, this);
        }
        m_toolTipDisappearTimer.stop();
    }

    m_hoverIndicator->setTargetItem(applet);
    if (!m_hoverIndicator->isVisible()) {
        m_hoverIndicator->setGeometry(applet->geometry());
        m_hoverIndicator->show();
    }
}

void AppletsListWidget::appletIconHoverLeave(AppletIconWidget *applet)
{
    Q_UNUSED(applet)

    if (m_toolTip->isVisible()) {
        m_toolTipDisappearTimer.start(TOOLTIP_DISAPPEAR_DELAY, this);
    } else {
        m_toolTipAppearTimer.stop();
    }
}

void AppletsListWidget::onToolTipEnter()
{
    m_toolTipDisappearTimer.stop();
}

void AppletsListWidget::onToolTipLeave()
{
    m_toolTipDisappearTimer.start(TOOLTIP_DISAPPEAR_DELAY, this);
}

void AppletsListWidget::setToolTipPosition()
{
    QPointF appletPosition = m_toolTip->appletIconWidget()->mapToItem(this, 0, 0);
    QRectF appletRect = m_toolTip->appletIconWidget()->
                        mapRectToItem(this, m_toolTip->appletIconWidget()->boundingRect());

    toolTipMoveFrom = m_toolTip->pos();

    Plasma::Corona *corona = static_cast<Plasma::WidgetExplorer*>(parentItem())->corona();
    if (corona) {
        toolTipMoveTo = corona->popupPosition(m_toolTip->appletIconWidget(), m_toolTip->geometry().size());
    } else {
        toolTipMoveTo = QPoint(appletPosition.x(), appletPosition.y());
    }

    if (m_toolTip->isVisible()) {
        animateToolTipMove();
    } else {
        m_toolTip->move(toolTipMoveTo);
    } 
}

void AppletsListWidget::insertAppletIcon(AppletIconWidget *appletIconWidget)
{
    if (appletIconWidget != 0) {
        appletIconWidget->setVisible(true);
        appletIconWidget->setParentItem(this);
        m_appletListLinearLayout->addItem(appletIconWidget);
        m_appletListLinearLayout->setAlignment(appletIconWidget, Qt::AlignHCenter);
    }
}

int AppletsListWidget::maximumAproxVisibleIconsOnList()
{
    qreal windowSize;
    qreal listTotalSize;
    qreal iconAverageSize;
    qreal maxVisibleIconsOnList;

    if (m_orientation == Qt::Horizontal) {
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

AppletIconWidget *AppletsListWidget::createAppletIcon(PlasmaAppletItem *appletItem)
{
    AppletIconWidget *applet = new AppletIconWidget(0, appletItem);
    applet->setMinimumSize(100, 0);
    qreal l, t, r, b;
    m_hoverIndicator->getContentsMargins(&l, &t, &r, &b);
    applet->setContentsMargins(l, t, r, b);

    connect(applet, SIGNAL(hoverEnter(AppletIconWidget*)), this, SLOT(appletIconHoverEnter(AppletIconWidget*)));
    connect(applet, SIGNAL(hoverLeave(AppletIconWidget*)), this, SLOT(appletIconHoverLeave(AppletIconWidget*)));
    connect(applet, SIGNAL(selected(AppletIconWidget*)), this, SLOT(itemSelected(AppletIconWidget*)));
    connect(applet, SIGNAL(doubleClicked(AppletIconWidget*)), this, SLOT(appletIconDoubleClicked(AppletIconWidget*)));

    return applet;
}

void AppletsListWidget::itemSelected(AppletIconWidget *applet)
{
    if (m_selectedItem) {
        m_selectedItem->setSelected(false);
    }

    applet->setSelected(true);
    m_selectedItem = applet;
    m_selectionIndicator->setTargetItem(m_selectedItem);
}

void AppletsListWidget::appletIconDoubleClicked(AppletIconWidget *applet)
{
    emit(appletDoubleClicked(applet->appletItem()));
}

void AppletsListWidget::eraseList()
{
    QList<QGraphicsItem *> applets = m_appletsListWidget->childItems();
    foreach (QGraphicsItem *applet, applets) {
        applet->setParentItem(0);
        applet->setVisible(false);
    }
}

void AppletsListWidget::updateList()
{
    AbstractItem *item;
    PlasmaAppletItem *appletItem;
    AppletIconWidget *appletIconWidget;

    m_appletsListWidget->setLayout(NULL);
    m_appletListLinearLayout = new QGraphicsLinearLayout(m_orientation);
    m_appletsListWidget->resize(0,0);
    m_currentAppearingAppletsOnList->clear();

    eraseList();

    for (int i = 0; i < m_modelFilterItems->rowCount(); i++) {
        item = getItemByProxyIndex(m_modelFilterItems->index(i, 0));
        appletItem = (PlasmaAppletItem*) item;

        if (appletItem != 0) {
            appletIconWidget = m_allAppletsHash.value(appletItem->id());
            insertAppletIcon(appletIconWidget);
            m_currentAppearingAppletsOnList->append(appletIconWidget);
        }
    }

    m_appletsListWidget->setLayout(m_appletListLinearLayout);
    m_appletListLinearLayout->setSpacing(0);

    updateGeometry();

    m_hoverIndicator->hide();

    resetScroll();
}

void AppletsListWidget::wheelEvent(QGraphicsSceneWheelEvent *event)
{
    ScrollPolicy side;
    bool canScroll;

    side = event->delta() < 0 ? AppletsListWidget::DownRight : AppletsListWidget::UpLeft;

    if (side == AppletsListWidget::DownRight) {
        canScroll = m_downRightArrow->isEnabled();
    } else {
        canScroll = m_upLeftArrow->isEnabled();
    }

    if (canScroll) {
        scroll(side, AppletsListWidget::Wheel);
    }
}

void AppletsListWidget::onRightArrowPress()
{
    scroll(AppletsListWidget::DownRight, AppletsListWidget::Wheel);
}

void AppletsListWidget::onLeftArrowPress()
{
    scroll(AppletsListWidget::UpLeft, AppletsListWidget::Wheel);
}

void AppletsListWidget::scroll(ScrollPolicy side, ScrollPolicy how)
{
    QRectF visibleRect = visibleListRect();
    int step = (how == AppletsListWidget::Wheel) ? wheelStep : arrowClickStep;

    m_toolTip->setVisible(false);

    switch (side) {
        case AppletsListWidget::DownRight:
            scrollDownRight(step, visibleRect);
            break;
        case AppletsListWidget::UpLeft:
            scrollUpLeft(step, visibleRect);
            break;
        default:
            break;
    }
}

void AppletsListWidget::scrollDownRight(int step, QRectF visibleRect)
{
    qreal scrollAmount;
    int lastVisiblePositionOnList;
    int lastVisibleItemIndex;
    AppletIconWidget *newLastIcon;
    int appletsIndexesToSum;
    qreal listSize;

    if (m_orientation == Qt::Horizontal) {
        lastVisiblePositionOnList = visibleRect.x() + visibleRect.width();
    } else {
        lastVisiblePositionOnList = visibleRect.y() + visibleRect.height();
    }

    lastVisibleItemIndex = findLastVisibleApplet(lastVisiblePositionOnList);

    //find out if the last icon on list appears clipped
    if (isItemUnder(lastVisibleItemIndex, lastVisiblePositionOnList)) {
        appletsIndexesToSum = step - 1;
    } else {
        appletsIndexesToSum = step;
    }

    //find out if the step is more than necessary to reach the end of the list
    if (lastVisibleItemIndex + appletsIndexesToSum >= m_currentAppearingAppletsOnList->count()) {
        appletsIndexesToSum = m_currentAppearingAppletsOnList->count() - 1 - lastVisibleItemIndex;
    }

    //who's gonna be the new last appearing icon on the list?!
    newLastIcon = m_currentAppearingAppletsOnList->at(lastVisibleItemIndex + appletsIndexesToSum);

    //scroll enough to show the new last icon on the list
    //and store the list size
    if (m_orientation == Qt::Horizontal) {
        scrollAmount = (newLastIcon->pos().x() + newLastIcon->boundingRect().size().width()) -
                        lastVisiblePositionOnList;
        listSize = m_appletsListWidget->boundingRect().size().width();
    } else {
        scrollAmount = (newLastIcon->pos().y() + newLastIcon->size().height()) -
                        lastVisiblePositionOnList;
        listSize = m_appletsListWidget->boundingRect().size().height();
    }

    //if the newLastIcon is the actual last icon on list, scroll until the end of the list
    if(lastVisibleItemIndex + appletsIndexesToSum == m_currentAppearingAppletsOnList->count() - 1) {
        scrollAmount = listSize - lastVisiblePositionOnList;
    }

    //do both:
    //if the newLastIcon is the actual last icon on list, scroll until the end of the list;
    //check if the scrollAmount is more than necessary to reach the end of the list
    if ((lastVisibleItemIndex + appletsIndexesToSum == m_currentAppearingAppletsOnList->count() - 1)  ||
        (lastVisiblePositionOnList + scrollAmount > listSize)) {
        scrollAmount = listSize - lastVisiblePositionOnList;
    }

    animateMoveBy(-scrollAmount);
    emit(listScrolled());
}

void AppletsListWidget::scrollUpLeft(int step, QRectF visibleRect)
{
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

    //find out if the last icon on list appears clipped
    if(isItemUnder(firstVisibleItemIndex, firstVisiblePositionOnList)) {
        appletsIndexesToReduce = step - 1;
    } else {
        appletsIndexesToReduce = step;
    }

    //find out if the step is more than necessary to reach the begining of the list
    if(firstVisibleItemIndex - appletsIndexesToReduce < 0) {
        appletsIndexesToReduce = firstVisibleItemIndex;
    }

    //who's gonna be the new first appearing icon on the list?!
    newFirstIcon = m_currentAppearingAppletsOnList->at(firstVisibleItemIndex - appletsIndexesToReduce);

    //scroll enough to show the new last icon on the list
    if(m_orientation == Qt::Horizontal) {
        scrollAmount = firstVisiblePositionOnList - newFirstIcon->pos().x();
    } else {
        scrollAmount = firstVisiblePositionOnList - newFirstIcon->pos().y();
    }

    //do both:
    //if the newFirstIcon is the actual first icon on list, scroll until the beggining of the list
    //check if the scrollAmount is more than necessary to reach the begining of the list
    if((firstVisibleItemIndex - appletsIndexesToReduce == 0) ||
       (firstVisiblePositionOnList - scrollAmount <  0)){
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
void AppletsListWidget::animateMoveBy(int amount)
{
    if (m_orientation == Qt::Horizontal) {
        m_scrollFrom = m_appletsListWidget->pos().x();
    } else {
        m_scrollFrom = m_appletsListWidget->pos().y();
    }

    if (m_scrollTimeLine.state() == QTimeLine::Running) {
        m_scrollTo = m_scrollTo + amount;
    } else {
        m_scrollTo = m_scrollFrom + amount;
    }

    if (m_scrollTo > 0) {
        m_scrollTo = 0;
    }

    if (m_orientation == Qt::Horizontal) {
        if (m_scrollTo + m_appletsListWidget->size().width() <
            m_appletsListWindowWidget->size().width()) {
            m_scrollTo = m_appletsListWindowWidget->size().width()
                         - m_appletsListWidget->size().width();
        }
    } else {
        if (m_scrollTo + m_appletsListWidget->size().height() <
            m_appletsListWindowWidget->size().height()) {
            m_scrollTo = m_appletsListWindowWidget->size().height()
                         - m_appletsListWidget->size().height();
        }
    }

    if (m_scrollTimeLine.state() != QTimeLine::Running && m_scrollFrom != m_scrollTo) {
        m_scrollTimeLine.start();
    }
}

void AppletsListWidget::animateToolTipMove()
{
    if (toolTipMoveTimeLine.state() != QTimeLine::Running && toolTipMoveFrom != toolTipMoveTo) {
        toolTipMoveTimeLine.start();
    }
}

void AppletsListWidget::scrollTimeLineFrameChanged(int frame)
{
    QPointF newPos;

    if (m_orientation == Qt::Horizontal) {
        newPos = QPointF(
                (frame/(qreal)100) * (m_scrollTo - m_scrollFrom) + m_scrollFrom,
                m_appletsListWidget->pos().y());

    } else {
        newPos = QPointF(
                m_appletsListWidget->pos().x(),
                (frame/(qreal)100) * (m_scrollTo - m_scrollFrom) + m_scrollFrom);
    }

    m_appletsListWidget->setPos(newPos);

    if (frame == m_scrollTimeLine.endFrame()) {
        manageArrows();

        if (m_scrollTo < m_scrollFrom) {
            // scrolling to the right/down
            if (m_downRightArrow->isEnabled() && m_downRightArrow->isDown()) {
                QTimer::singleShot(0, this, SLOT(onRightArrowPress()));
            }
        } else if (m_upLeftArrow->isEnabled() && m_upLeftArrow->isDown()) {
            // scroll left/up
            QTimer::singleShot(0, this, SLOT(onLeftArrowPress()));
        }
    }
}

void AppletsListWidget::toolTipMoveTimeLineFrameChanged(int frame)
{
    QPoint newPos;

    newPos = QPoint(
            (frame/(qreal)100) * (toolTipMoveTo.x() - toolTipMoveFrom.x()) + toolTipMoveFrom.x(),
            (frame/(qreal)100) * (toolTipMoveTo.y() - toolTipMoveFrom.y()) + toolTipMoveFrom.y());

    m_toolTip->move(newPos);
}

void AppletsListWidget::resetScroll()
{
    m_appletsListWidget->setPos(0,0);
    manageArrows();
}

void AppletsListWidget::manageArrows()
{
    QRectF visibleRect = visibleListRect();
    qreal windowSize;
    qreal listSize;

    qDebug() << "manage the arrows";

    if (m_orientation == Qt::Horizontal) {
        windowSize = m_appletsListWindowWidget->geometry().width();
        listSize = m_appletsListWidget->size().width();
    } else {
        windowSize = m_appletsListWindowWidget->geometry().height();
        listSize = m_appletsListWidget->size().height();
    }

    if (listSize <= windowSize || m_currentAppearingAppletsOnList->count() == 0) {
        m_upLeftArrow->setEnabled(false);
        m_downRightArrow->setEnabled(false);
    } else {
        qreal firstVisiblePositionOnList;
        qreal lastVisiblePositionOnList;

        if (m_orientation == Qt::Horizontal) {
            firstVisiblePositionOnList = visibleRect.x();
            lastVisiblePositionOnList = firstVisiblePositionOnList + visibleRect.width();
        } else {
            firstVisiblePositionOnList = visibleRect.y();
            lastVisiblePositionOnList = firstVisiblePositionOnList + visibleListRect().height();
        }

        if (firstVisiblePositionOnList <= m_appletListLinearLayout->spacing()) {
            m_upLeftArrow->setEnabled(false);
        } else {
            m_upLeftArrow->setEnabled(true);
        }

        if (lastVisiblePositionOnList >= listSize - m_appletListLinearLayout->spacing()) {
            m_downRightArrow->setEnabled(false);
        } else {
            m_downRightArrow->setEnabled(true);
        }
    }
}

bool AppletsListWidget::isItemUnder(int itemIndex, qreal position)
{
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

int AppletsListWidget::findFirstVisibleApplet(int firstVisiblePositionOnList) {

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

int AppletsListWidget::findLastVisibleApplet(int lastVisiblePositionOnList) {

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

QRectF AppletsListWidget::visibleListRect() {
    QRectF visibleRect = m_appletsListWindowWidget->
                         mapRectToItem(m_appletsListWidget, 0, 0,
                                          m_appletsListWindowWidget->geometry().width(),
                                          m_appletsListWindowWidget->geometry().height());

    return visibleRect;
}

void AppletsListWidget::populateAllAppletsHash()
{
    AbstractItem *item;
    PlasmaAppletItem *appletItem;
    int indexesCount = m_modelFilterItems->rowCount();

    qDeleteAll(m_allAppletsHash);
    m_allAppletsHash.clear();

    for (int i = 0; i < indexesCount ; i++){
        item = getItemByProxyIndex(m_modelFilterItems->index(i, 0));
        appletItem = (PlasmaAppletItem*) item;
        m_allAppletsHash.insert(appletItem->id(), createAppletIcon(appletItem));
    }
}

AbstractItem *AppletsListWidget::getItemByProxyIndex(const QModelIndex &index) const
{
    return (AbstractItem *)m_modelItems->itemFromIndex(m_modelFilterItems->mapToSource(index));
}

QList <AbstractItem *> AppletsListWidget::selectedItems() const
{
//    return m_appletList->selectedItems();
    return QList<AbstractItem *>();
}
