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

#include <Plasma/Containment>
#include <Plasma/Corona>
#include <Plasma/Theme>

#include "widgetexplorer.h" //FIXME really? :/

const int FILTER_APPLIANCE_DELAY = 400;
const int TOOLTIP_APPEAR_DELAY = 1000;
const int TOOLTIP_APPEAR_WHEN_VISIBLE_DELAY = 300;
const int TOOLTIP_DISAPPEAR_DELAY = 300;

using namespace KCategorizedItemsViewModels;

AppletsListWidget::AppletsListWidget(Plasma::Location location, QGraphicsItem *parent)
    : AbstractIconList(location, parent)
{
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
}

AppletsListWidget::~AppletsListWidget()
{
    delete m_toolTip;
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
    connect(m_modelItems, SIGNAL(rowsAboutToBeRemoved(const QModelIndex&, int, int)),
            this, SLOT(rowsAboutToBeRemoved(const QModelIndex&, int, int)));

    updateList();
}

void AppletsListWidget::setFilterModel(QStandardItemModel *model)
{
    m_modelFilters = model;
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

void AppletsListWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_toolTipAppearTimer.timerId()) {
        m_toolTipAppearTimer.stop();
        m_toolTip->updateContent();
        m_toolTip->syncToGraphicsWidget();
        setToolTipPosition();
        m_toolTip->setVisible(true);
    } else if (event->timerId() == m_toolTipAppearWhenAlreadyVisibleTimer.timerId()) {
        m_toolTipAppearWhenAlreadyVisibleTimer.stop();
        m_toolTip->updateContent();
        m_toolTip->syncToGraphicsWidget();
        setToolTipPosition();
    } else if (event->timerId() == m_toolTipDisappearTimer.timerId()) {
        m_toolTipDisappearTimer.stop();
        m_toolTip->setVisible(false);
    } else if (event->timerId() == m_filterApplianceTimer.timerId()) {
        m_filterApplianceTimer.stop();
        m_modelFilterItems->setFilter(qVariantValue<KCategorizedItemsViewModels::Filter>
                                      (m_dataFilterAboutToApply));
    }

    QGraphicsWidget::timerEvent(event);
}

QVariant AppletsListWidget::itemChange(GraphicsItemChange change, const QVariant & value)
{
    if (change == QGraphicsItem::ItemSceneHasChanged) {
        m_toolTip->setScene(scene());
    }

    return QGraphicsWidget::itemChange(change, value);
}


void AppletsListWidget::appletIconHoverEnter(Plasma::AbstractIcon *icon)
{
    AppletIconWidget *applet = static_cast<AppletIconWidget*>(icon);
    if (!m_toolTip->isVisible()) {
        m_toolTip->setAppletIconWidget(applet);
        m_toolTipAppearTimer.start(TOOLTIP_APPEAR_DELAY, this);
    } else {
        if(m_toolTip->appletIconWidget()->appletItem() &&
            !(m_toolTip->appletIconWidget()->appletItem()->pluginName() ==
            applet->appletItem()->pluginName())) {
            m_toolTip->setAppletIconWidget(applet);

            //small delay, so if one's hovering very fast over the icons,
            //the tooltip doesn't appear frantically
            m_toolTipAppearWhenAlreadyVisibleTimer.start(TOOLTIP_APPEAR_WHEN_VISIBLE_DELAY, this);
        }
        m_toolTipDisappearTimer.stop();
    }
}

void AppletsListWidget::appletIconHoverLeave(Plasma::AbstractIcon *icon)
{
    Q_UNUSED(icon)

    if (m_toolTip->isVisible()) {
        m_toolTipDisappearTimer.start(TOOLTIP_DISAPPEAR_DELAY, this);
    } else {
        m_toolTipAppearTimer.stop();
    }
}

void AppletsListWidget::appletIconDragging(Plasma::AbstractIcon *icon)
{
    Q_UNUSED(icon)
    m_toolTip->hide();
    m_toolTipAppearTimer.stop();
    m_toolTipDisappearTimer.stop();
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
        toolTipMoveTo = corona->popupPosition(m_toolTip->appletIconWidget(),
                                              m_toolTip->geometry().size(),
                                              Qt::AlignCenter);
    } else {
        toolTipMoveTo = QPoint(appletPosition.x(), appletPosition.y());
    }

    if (location() == Plasma::LeftEdge || location() == Plasma::RightEdge) {
        toolTipMoveFrom.setX(toolTipMoveTo.x());
    } else {
        toolTipMoveFrom.setY(toolTipMoveTo.y());
    }

    m_toolTip->move(toolTipMoveTo);

    if (m_toolTip->isVisible()) {
        animateToolTipMove();
    } else {
        m_toolTip->move(toolTipMoveTo);
    }
}

AppletIconWidget *AppletsListWidget::createAppletIcon(PlasmaAppletItem *appletItem)
{
    AppletIconWidget *applet = new AppletIconWidget(appletItem);
    addIcon(applet);

    connect(applet, SIGNAL(hoverEnter(Plasma::AbstractIcon*)), this, SLOT(appletIconHoverEnter(Plasma::AbstractIcon*)));
    connect(applet, SIGNAL(hoverLeave(Plasma::AbstractIcon*)), this, SLOT(appletIconHoverLeave(Plasma::AbstractIcon*)));
    connect(applet, SIGNAL(dragging(Plasma::AbstractIcon*)), this, SLOT(appletIconDragging(Plasma::AbstractIcon*)));
    connect(applet, SIGNAL(doubleClicked(Plasma::AbstractIcon*)), this, SLOT(appletIconDoubleClicked(Plasma::AbstractIcon*)));
    //FIXME no such signal, needs implementing?
    //connect(applet, SIGNAL(dragStarted(AbstractIcon*)), m_toolTip, SLOT(hide()));

    return applet;
}

void AppletsListWidget::appletIconDoubleClicked(Plasma::AbstractIcon *icon)
{
    emit(appletDoubleClicked(static_cast<AppletIconWidget*>(icon)->appletItem()));
}

void AppletsListWidget::updateVisibleIcons()
{
    m_toolTip->setVisible(false); // hides possibly open tooltip when list updates

    //not sure if this is the fastest way or not; depends on the speed of model-view stuff
    hideAllIcons();

    //insert items that match the filter
    for (int i = 0; i < m_modelFilterItems->rowCount(); i++) {
        PlasmaAppletItem *appletItem = static_cast<PlasmaAppletItem*>(getItemByProxyIndex(m_modelFilterItems->index(i, 0)));

        //FIXME the contains check may be redundant?
        if (appletItem && m_allAppletsHash.contains(appletItem->id())) {
            Plasma::AbstractIcon *appletIconWidget = m_allAppletsHash.value(appletItem->id());
            showIcon(appletIconWidget);
        }
    }

}

void AppletsListWidget::animateToolTipMove()
{
    if (toolTipMoveTimeLine.state() != QTimeLine::Running && toolTipMoveFrom != toolTipMoveTo) {
         toolTipMoveTimeLine.start();
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

void AppletsListWidget::populateAllAppletsHash()
{
    qDeleteAll(m_allAppletsHash);
    m_allAppletsHash.clear();
//FIXME only the ones matching the filter? okay, the filter matches everything at the start but
//this still feels Wrong.
    const int indexesCount = m_modelFilterItems->rowCount();
    for (int i = 0; i < indexesCount ; i++) {
        PlasmaAppletItem *appletItem = static_cast<PlasmaAppletItem*>(getItemByProxyIndex(m_modelFilterItems->index(i, 0)));
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
//FIXME kill this after removing deps
    return QList<AbstractItem *>();
}

//when the crap is *this* called?
void AppletsListWidget::rowsAboutToBeRemoved(const QModelIndex& parent, int row, int column)
{
    Q_UNUSED(parent)
    Q_UNUSED(column)
    PlasmaAppletItem *item = dynamic_cast<PlasmaAppletItem*>(m_modelItems->item(row));
    if (item) {
        m_toolTip->hide();
        m_toolTip->setAppletIconWidget(0);
        m_allAppletsHash.remove(item->id());
        item->deleteLater();
        updateList();
    }
}

void AppletsListWidget::setSearch(const QString &searchString)
{
    m_modelFilterItems->setSearch(searchString);
}

void AppletsListWidget::scrollTo(int index)
{
    m_toolTip->setVisible(false);
    AbstractIconList::scrollTo(index);
}


