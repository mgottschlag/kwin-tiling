/***************************************************************************
 *   Copyright (C) 2010 by Ingomar Wesp <ingomar@wesp.name>                *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301,  USA.         *
 ***************************************************************************/
#include "icongrid.h"

// Qt
#include <Qt>
#include <QtCore/QEvent>
#include <QtCore/QMimeData>
#include <QtCore/QPointF>
#include <QtCore/QRectF>
#include <QtCore/QUrl>
#include <QtGui/QApplication>
#include <QtGui/QBrush>
#include <QtGui/QDrag>
#include <QtGui/QGraphicsGridLayout>
#include <QtGui/QGraphicsItem>
#include <QtGui/QGraphicsLinearLayout>
#include <QtGui/QGraphicsSceneDragDropEvent>
#include <QtGui/QGraphicsSceneMouseEvent>
#include <QtGui/QGraphicsSceneResizeEvent>
#include <QtGui/QGraphicsWidget>
#include <QtGui/QIcon>
#include <QtGui/QPainter>
#include <QtGui/QStyleOptionGraphicsItem>
#include <QtGui/QWidget>

// KDE
#include <KIcon>
#include <KUrl>

// Plasma
#include <Plasma/Plasma>
#include <Plasma/IconWidget>
#include <Plasma/Theme>
#include <Plasma/ToolTipContent>
#include <Plasma/ToolTipManager>

// stdlib
#include <math.h>

// Own
#include "icongridlayout.h"
#include "itemdata.h"
#include "quicklaunchicon.h"

using Plasma::Theme;

namespace Quicklaunch {

class DropMarker : public QuicklaunchIcon {

public:
    DropMarker(IconGrid *parent)
        : QuicklaunchIcon(ItemData(), parent)
    {
        setOwnedByLayout(false);
        hide();
    }

protected:
    void paint(
        QPainter *painter,
        const QStyleOptionGraphicsItem *option,
        QWidget *widget)
    {
        // This mirrors the behavior of the panel spacer committed by mart
        // workspace/plasma/desktop/containments/panel/panel.cpp R875513)
        QColor brushColor(Theme::defaultTheme()->color(Theme::TextColor));
        brushColor.setAlphaF(0.3);

        painter->setRenderHint(QPainter::Antialiasing);
        painter->setPen(Qt::NoPen);
        painter->setBrush(QBrush(brushColor));

        painter->drawRoundedRect(contentsRect(), 4, 4);
        QuicklaunchIcon::paint(painter, option, widget);
    }
};

IconGrid::IconGrid(QGraphicsItem *parent)
:
    QGraphicsWidget(parent),
    m_icons(),
    m_iconNamesVisible(false),
    m_locked(false),
    m_layout(new IconGridLayout()),
    m_mousePressedPos(),
    m_dropMarker(0),
    m_dropMarkerIndex(-1),
    m_placeHolder(0)
{
    m_dropMarker = new DropMarker(this);

    setLayout(m_layout);
    initPlaceHolder();

    setLocked(false);
}

bool IconGrid::iconNamesVisible() const
{
    return m_iconNamesVisible;
}

void IconGrid::setIconNamesVisible(bool enable)
{
    if (enable == m_iconNamesVisible) {
        return;
    }

    Q_FOREACH (QuicklaunchIcon *icon, m_icons) {
        icon->setIconNameVisible(enable);
    }
    m_dropMarker->setIconNameVisible(enable);
    m_iconNamesVisible = enable;
}

bool IconGrid::locked() const
{
    return m_locked;
}

void IconGrid::setLocked(bool enable)
{
    m_locked = enable;
    setAcceptDrops(!enable);
}

IconGridLayout * IconGrid::layout()
{
    return m_layout;
}

int IconGrid::iconCount() const
{
    return m_icons.size();
}

void IconGrid::insert(int index, const ItemData &itemData)
{
    QList<ItemData> itemDataList;
    itemDataList.append(itemData);
    insert(index, itemDataList);
}

void IconGrid::insert(int index, const QList<ItemData> &itemDataList)
{
    if (itemDataList.size() == 0) {
        return;
    }

    if (m_icons.size() == 0 && m_placeHolder) {
        deletePlaceHolder();
        index = 0;
    }
    else if (index < 0 || index > m_icons.size()) {
        index = m_icons.size();
    }

    Q_FOREACH(ItemData itemData, itemDataList) {

        QuicklaunchIcon *icon = new QuicklaunchIcon(itemData);
        icon->setIconNameVisible(m_iconNamesVisible);
        icon->setOrientation(Qt::Vertical);
        icon->installEventFilter(this);
        connect(icon, SIGNAL(clicked()), this, SIGNAL(iconClicked()));

        m_icons.insert(index, icon);

        if (m_dropMarkerIndex != -1 && m_dropMarkerIndex <= index) {
            m_layout->insertItem(index+1, icon);
        }
        else {
            m_layout->insertItem(index, icon);
        }

        index++;
    }

    Q_EMIT iconsChanged();
    Q_EMIT displayedItemCountChanged();
}

void IconGrid::removeAt(int index)
{
    if (m_dropMarkerIndex != -1 && m_dropMarkerIndex <= index) {
        m_layout->removeAt(index+1);
    }
    else {
        m_layout->removeAt(index);
    }

    m_icons.removeAt(index);

    if (m_icons.size() == 0) {
        initPlaceHolder();
    }

    Q_EMIT iconsChanged();
    Q_EMIT displayedItemCountChanged();
}

ItemData IconGrid::iconAt(int index) const
{
    return m_icons.at(index)->itemData();
}

int IconGrid::iconIndexAtPosition(const QPointF& pos) const
{
    for (int i = 0; i < m_icons.size(); i++) {
        if (m_icons.at(i)->geometry().contains(pos)) {
            return i;
        }
    }
    return -1;
}

bool IconGrid::eventFilter(QObject *watched, QEvent *event)
{
    QuicklaunchIcon *quicklaunchIconSource =
        qobject_cast<QuicklaunchIcon*>(watched);

    // Start of Drag & Drop operations
    if (quicklaunchIconSource && !m_locked) {

        if (event->type() == QEvent::GraphicsSceneMousePress) {
            m_mousePressedPos =
                static_cast<QGraphicsSceneMouseEvent*>(event)->pos();

            return false;
        }
        else if (event->type() == QEvent::GraphicsSceneMouseMove) {

            QGraphicsSceneMouseEvent *mouseEvent =
                static_cast<QGraphicsSceneMouseEvent*>(event);

            if ((m_mousePressedPos - mouseEvent->pos()).manhattanLength() >=
                    QApplication::startDragDistance()) {

                ItemData sourceData = quicklaunchIconSource->itemData();

                QMimeData *mimeData = new QMimeData();
                sourceData.populateMimeData(mimeData);

                QDrag *drag = new QDrag(mouseEvent->widget());
                drag->setMimeData(mimeData);
                drag->setPixmap(quicklaunchIconSource->icon().pixmap(16, 16));

                int iconIndex = m_icons.indexOf(quicklaunchIconSource);

                removeAt(iconIndex);

                Qt::DropAction dropAction = drag->exec();

                if (dropAction != Qt::MoveAction) {
                    // Restore the icon.
                    insert(iconIndex, sourceData);
                }
                return true;
            }
        }
    }

    return false;
}

void IconGrid::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_ASSERT(!m_locked);

    Qt::DropAction proposedAction = event->proposedAction();

    if (proposedAction != Qt::CopyAction && proposedAction != Qt::MoveAction) {

        Qt::DropActions possibleActions = event->possibleActions();

        if (possibleActions & Qt::CopyAction) {
            event->setProposedAction(Qt::CopyAction);
        }
        else if (possibleActions & Qt::MoveAction) {
            event->setProposedAction(Qt::MoveAction);
        } else {
            event->setAccepted(false);
            return;
        }
    }

    // Initialize drop marker
    const QMimeData *mimeData = event->mimeData();

    if (ItemData::canDecode(mimeData)) {
        QList<ItemData> data = ItemData::fromMimeData(mimeData);

        if (data.size() > 0) {

            if (data.size() == 1) {
                m_dropMarker->setItemData(data.at(0));
            } else {
                m_dropMarker->setItemData(ItemData());
                m_dropMarker->setIcon("document-multiple");

                if (m_iconNamesVisible) {
                    m_dropMarker->setText(i18n("Multiple items"));
                } else {
                    m_dropMarker->setText(QString::null);
                }
            }

            if (m_icons.size() != 0) {

                m_dropMarkerIndex = determineDropMarkerIndex(
                    mapFromScene(event->scenePos()));

            } else {
                deletePlaceHolder();
                m_dropMarkerIndex = 0;
            }

            m_layout->insertItem(m_dropMarkerIndex, m_dropMarker);
            m_dropMarker->show();

            if (m_icons.size() != 0)
                Q_EMIT displayedItemCountChanged();

            event->accept();
        } else {
            event->setAccepted(false);
            return;
        }
    } else {
        event->setAccepted(false);
        return;
    }
}

void IconGrid::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    // DragMoveEvents are always preceded by DragEnterEvents
    Q_ASSERT(m_dropMarkerIndex != -1);

    int newDropMarkerIndex =
        determineDropMarkerIndex(mapFromScene(event->scenePos()));

    if (newDropMarkerIndex != m_dropMarkerIndex) {
        m_layout->moveItem(m_dropMarkerIndex, newDropMarkerIndex);
        m_dropMarkerIndex = newDropMarkerIndex;
    }
}

void IconGrid::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    if (m_dropMarkerIndex != -1) {

        m_dropMarker->hide();
        m_layout->removeAt(m_dropMarkerIndex);
        m_dropMarker->setItemData(ItemData());
        m_dropMarkerIndex = -1;

        if (m_icons.size() == 0) {
            initPlaceHolder();
        }

        Q_EMIT displayedItemCountChanged();
    }
}

void IconGrid::dropEvent(QGraphicsSceneDragDropEvent *event)
{
    int dropIndex = m_dropMarkerIndex;

    if (dropIndex != -1) {
        m_dropMarker->hide();
        m_layout->removeAt(m_dropMarkerIndex);
        m_dropMarker->setItemData(ItemData());
        m_dropMarkerIndex = -1;
    }

    const QMimeData *mimeData = event->mimeData();

    if (ItemData::canDecode(mimeData)) {

        QList<ItemData> data = ItemData::fromMimeData(mimeData);
        insert(dropIndex, data);
    }

    event->accept();
}

void IconGrid::onPlaceHolderActivated() {

    Q_ASSERT(m_placeHolder);
    Plasma::ToolTipManager::self()->show(m_placeHolder);
}

void IconGrid::initPlaceHolder() {

    Q_ASSERT(!m_placeHolder);

    m_placeHolder = new Plasma::IconWidget(KIcon("fork"), QString(), this);

    Plasma::ToolTipContent tcp(
        i18n("Quicklaunch"),
        i18n("Add launchers by Drag and Drop or by using the context menu."),
        m_placeHolder->icon());
    Plasma::ToolTipManager::self()->setContent(m_placeHolder, tcp);

    connect(m_placeHolder, SIGNAL(activated()), SLOT(onPlaceHolderActivated()));
    m_layout->addItem(m_placeHolder);
}

void IconGrid::deletePlaceHolder() {

    Q_ASSERT(m_placeHolder);

    m_layout->removeAt(0);

    delete m_placeHolder;
    m_placeHolder = 0;
}

int IconGrid::determineDropMarkerIndex(const QPointF &localPos) const
{
    if (m_placeHolder) {
        return 0;
    }

    // Determine the new index of the drop marker.
    const int rowCount = m_layout->rowCount();
    const int columnCount = m_layout->columnCount();

    int row = 0;
    while (row + 1 < rowCount && localPos.y() > m_layout->itemAt(row + 1, 0)->geometry().top()) {
        row++;
    }

    int col = 0;
    while (col + 1 < columnCount && localPos.x() > m_layout->itemAt(0, col + 1)->geometry().left()) {
        col++;
    }

    int newDropMarkerIndex = row * columnCount + col;

    if (newDropMarkerIndex > m_icons.size())
        newDropMarkerIndex = m_icons.size();

    return newDropMarkerIndex;
}
}

#include "icongrid.moc"
