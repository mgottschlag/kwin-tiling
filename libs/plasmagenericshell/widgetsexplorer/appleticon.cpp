/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
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

#include "appleticon.h"

#include <kiconloader.h>

AppletIconWidget::AppletIconWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem)
    : Plasma::IconWidget(parent)
{
    m_appletItem = appletItem;
    m_hovered = false;
    m_selected = false;
    m_selectedBackgroundSvg = new Plasma::FrameSvg(this);
    m_selectedBackgroundSvg->setImagePath("widgets/translucentbackground");

    updateApplet(appletItem);
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
    if (appletItem != 0) {
        m_appletItem = appletItem;
        setText(m_appletItem->name());
        setIcon(m_appletItem->icon());
    } else {
        setText("no name");
        setIcon("widgets/clock");
    }
}

void AppletIconWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *event)
{
    Plasma::IconWidget::hoverEnterEvent(event);
    m_hovered = true;
    emit(hoverEnter(this));
}

void AppletIconWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *event)
{
    Plasma::IconWidget::hoverLeaveEvent(event);
    m_hovered = false;
    emit(hoverLeave(this));
}

void AppletIconWidget::mouseMoveEvent(QGraphicsSceneMouseEvent * event)
{
    Plasma::IconWidget::mouseMoveEvent(event);
    if (event->button() != Qt::LeftButton
        && (event->pos() - event->buttonDownPos(Qt::LeftButton)).toPoint().manhattanLength() > QApplication::startDragDistance()) {
        event->accept();
        qDebug() << "Start Dragging";
        QDrag *drag = new QDrag(event->widget());
        QPixmap p = appletItem()->icon().pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
        drag->setPixmap(p);

        QMimeData *data = m_appletItem->mimeData();

        drag->setMimeData(data);
        drag->exec();

        mouseReleaseEvent(event);
    }
}

void AppletIconWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit(selected(this));
}

void AppletIconWidget::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    emit(doubleClicked(this));
}

void AppletIconWidget::setSelected(bool selected)
{
    m_selected = selected;
    update(0,0,boundingRect().width(), boundingRect().height());
}

void AppletIconWidget::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
 {
    if (m_selected || m_hovered) {
        m_selectedBackgroundSvg->resizeFrame(boundingRect().size());
        m_selectedBackgroundSvg->paintFrame(painter, boundingRect().topLeft());
        if (m_selected) {
            //again
            m_selectedBackgroundSvg->paintFrame(painter, boundingRect().topLeft());
        }
     }

    Plasma::IconWidget::paint(painter, option, widget);
 }
