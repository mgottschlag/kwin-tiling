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
#include "quicklaunchicon.h"
#include "icongridlayout.h"

using Plasma::Theme;

namespace Quicklaunch {

class DropMarker : public QuicklaunchIcon {

public:
    DropMarker(IconGrid *parent)
        : QuicklaunchIcon(KUrl(), parent)
    {
        setIcon(QIcon());
        setOwnedByLayout(false);
        hide();
    }

protected:
    void paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
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

IconGrid::IconGrid(
    Plasma::FormFactor formFactor,
    QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_icons(),
      m_iconNamesVisible(false),
      m_layout(new IconGridLayout()),
      m_mousePressedPos(),
      m_dropMarker(0),
      m_dropMarkerIndex(-1),
      m_placeHolder(0)
{
    m_dropMarker = new DropMarker(this);

    setLayout(m_layout);
    setFormFactor(formFactor);
    initPlaceHolder();

    setAcceptDrops(true);
}

bool IconGrid::iconNamesVisible()
{
    return m_iconNamesVisible;
}

void IconGrid::setIconNamesVisible(bool enable)
{
    if (enable == m_iconNamesVisible) {
        return;
    }

    if (enable) {
        Q_FOREACH (QuicklaunchIcon *icon, m_icons) {
            icon->setText(icon->appName());
        }
    } else {
        Q_FOREACH (QuicklaunchIcon *icon, m_icons) {
            icon->setText(QString());
        }
    }

    m_iconNamesVisible = enable;
}

int IconGrid::cellSpacing() const
{
    return m_layout->cellSpacing();
}

void IconGrid::setCellSpacing(int cellSpacing)
{
    m_layout->setCellSpacing(cellSpacing);
}

int IconGrid::maxRowsOrColumns() const
{
    return m_layout->maxRowsOrColumns();
}

void IconGrid::setMaxRowsOrColumns(int maxRowsOrColumns)
{
    m_layout->setMaxRowsOrColumns(maxRowsOrColumns);
}

bool IconGrid::maxRowsOrColumnsForced() const
{
    return m_layout->maxRowsOrColumnsForced();
}

void IconGrid::setMaxRowsOrColumnsForced(bool enable)
{
    m_layout->setMaxRowsOrColumnsForced(enable);
}

void IconGrid::setFormFactor(Plasma::FormFactor formFactor) {

    m_layout->setMode(
        formFactor == Plasma::Horizontal ?
            IconGridLayout::PreferRows :
            IconGridLayout::PreferColumns);

    // Ignore maxRowsOrColumns / maxRowsOrColumnsForced when in planar
    // form factor.
    if (formFactor == Plasma::Planar) {
        setMaxRowsOrColumns(0);
        setMaxRowsOrColumnsForced(false);
    }
}

int IconGrid::iconCount() const
{
    return m_icons.size();
}

int IconGrid::displayedItemCount() const
{
    return m_layout->count();
}

void IconGrid::insert(int index, const KUrl &url)
{
    if (m_icons.size() == 0 && m_placeHolder) {
        deletePlaceHolder();
        index = 0;
    }
    else if (index < 0 || index > m_icons.size()) {
        index = m_icons.size();
    }

    QuicklaunchIcon *icon = new QuicklaunchIcon(url, this);
    icon->installEventFilter(this);
    connect(icon, SIGNAL(clicked()), this, SIGNAL(iconClicked()));

    if (m_iconNamesVisible)
        icon->setText(icon->appName());

    m_icons.insert(index, icon);

    if (m_dropMarkerIndex != -1 && m_dropMarkerIndex <= index) {
        m_layout->insertItem(index+1, icon);
    }
    else {
        m_layout->insertItem(index, icon);
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

int IconGrid::indexOf(const KUrl &url, int from) const
{
    int index = -1;

    for (int i = from; i < m_icons.size(); i++) {
        if (m_icons.at(i)->url() == url) {
            index = i;
            break;
        }
    }

    return index;
}

KUrl IconGrid::iconAt(int index) const
{
    return m_icons.at(index)->url();
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

QList<KUrl> IconGrid::urls() const
{
    QList<KUrl> urls;

    Q_FOREACH(QuicklaunchIcon *icon, m_icons) {
        urls.append(icon->url());
    }

    return urls;
}

bool IconGrid::eventFilter(QObject *watched, QEvent *event)
{
    QuicklaunchIcon *quicklaunchIconSource =
        qobject_cast<QuicklaunchIcon*>(watched);

    if (quicklaunchIconSource) {

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

                QMimeData *mimeData = new QMimeData();

                QList<QUrl> urls;
                urls.append(quicklaunchIconSource->url());
                mimeData->setUrls(urls);

                QDrag *drag = new QDrag(mouseEvent->widget());
                drag->setMimeData(mimeData);
                drag->setPixmap(quicklaunchIconSource->icon().pixmap(16, 16));

                KUrl iconUrl = quicklaunchIconSource->url();
                int iconIndex = m_icons.indexOf(quicklaunchIconSource);

                removeAt(iconIndex);

                Qt::DropAction dropAction = drag->exec(
                    Qt::MoveAction | Qt::CopyAction);

                if (dropAction != Qt::MoveAction) {
                    // Restore the icon.
                    insert(iconIndex, iconUrl);
                }
                return true;
            }
        }
    }

    return false;
}

void IconGrid::dragEnterEvent(QGraphicsSceneDragDropEvent *event)
{
    dragMoveEvent(event);
}

void IconGrid::dragMoveEvent(QGraphicsSceneDragDropEvent *event)
{
    // Determine the new index of the drop marker.
    const QPointF point = mapFromScene(event->scenePos());

    const int rowCount = m_layout->rowCount();
    const int columnCount = m_layout->columnCount();

    int row = 0;
    while ((row + 1) < rowCount && point.y() > m_layout->itemAt(row + 1, 0)->geometry().top()) {
        row++;
    }

    int col = 0;
    while ((col + 1) < columnCount && point.x() > m_layout->itemAt(0, col + 1)->geometry().left()) {
        col++;
    }

    bool dropMarkerAdded = false;

    if (m_dropMarkerIndex == -1) {
        // Initialize drop marker
        const QMimeData *mimeData = event->mimeData();

        if (!mimeData->hasUrls() || mimeData->urls().size() == 0) {
            event->setAccepted(false);
            return;
        }

        QList<QUrl> urls = mimeData->urls();

        m_dropMarker->setUrl(urls.at(0));
        if (m_iconNamesVisible) {
            m_dropMarker->setText(m_dropMarker->appName());
        }

        dropMarkerAdded = true;
        event->accept();
    }

    int newDropMarkerIndex;

    if (dropMarkerAdded && m_icons.size() == 0) {
        deletePlaceHolder();
        newDropMarkerIndex = 0;
    }
    else {
        newDropMarkerIndex = row * columnCount + col;

        if (newDropMarkerIndex > m_icons.size())
            newDropMarkerIndex = m_icons.size();
    }

    if (newDropMarkerIndex != m_dropMarkerIndex) {
        if (dropMarkerAdded) {
            m_layout->insertItem(newDropMarkerIndex, m_dropMarker);
        } else {
            m_layout->moveItem(m_dropMarkerIndex, newDropMarkerIndex);
        }
        m_dropMarker->show();
        m_dropMarkerIndex = newDropMarkerIndex;
    }

    if (dropMarkerAdded) {
        Q_EMIT displayedItemCountChanged();
    }
}

void IconGrid::dragLeaveEvent(QGraphicsSceneDragDropEvent *event)
{
    Q_UNUSED(event);

    if (m_dropMarkerIndex != -1) {

        m_layout->removeAt(m_dropMarkerIndex);
        m_dropMarker->hide();
        m_dropMarker->clear();
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
        m_dropMarker->clear();
        m_layout->removeAt(m_dropMarkerIndex);
        m_dropMarkerIndex = -1;
    }

    if (event->mimeData()->hasUrls()) {

        QList<QUrl> urls = event->mimeData()->urls();
        Q_ASSERT(urls.size() > 0);

        insert(dropIndex, urls.at(0));
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
}

#include "icongrid.moc"
