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

#include <QFontMetrics>

#include <KIconLoader>
#include <KIcon>
#include <KGlobalSettings>

AppletIconWidget::AppletIconWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem)
    : QGraphicsWidget(parent),
      m_iconHeight(16)
{
    setFont(KGlobalSettings::smallestReadableFont());
    setAcceptHoverEvents(true);
    m_appletItem = appletItem;
    m_hovered = false;
    m_selected = false;
    m_selectedBackgroundSvg = new Plasma::FrameSvg(this);
    m_selectedBackgroundSvg->setImagePath("widgets/translucentbackground");
    qDebug() << cursor();
    setCursor(Qt::OpenHandCursor);
    m_runningIcon = KIcon("dialog-ok");
}

AppletIconWidget::~AppletIconWidget()
{
    m_appletItem = 0;
}

PlasmaAppletItem *AppletIconWidget::appletItem()
{
    return m_appletItem;
}

void AppletIconWidget::setIconHeight(int height)
{
    m_iconHeight = height;
    update();
}

void AppletIconWidget::setAppletItem(PlasmaAppletItem *appletIcon)
{
   m_appletItem = appletIcon;
   update();
}

void AppletIconWidget::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = true;
    emit(hoverEnter(this));
}

void AppletIconWidget::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = false;
    emit(hoverLeave(this));
}

void AppletIconWidget::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton &&
        (event->pos() - event->buttonDownPos(Qt::LeftButton)).toPoint().manhattanLength() > QApplication::startDragDistance()) {
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

void AppletIconWidget::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseReleaseEvent(event);
    setCursor(Qt::OpenHandCursor);
}

void AppletIconWidget::resizeEvent(QGraphicsSceneResizeEvent *)
{
    QFontMetrics fm(font());
    m_iconHeight = qBound(0, int(size().height() - fm.height()), int(size().height()));
    qDebug() << "icon height is" << m_iconHeight << size().height() << fm.height();
}

void AppletIconWidget::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    setCursor(Qt::ClosedHandCursor);
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
    Q_UNUSED(option)
    Q_UNUSED(widget)

    if (!m_appletItem) {
        return;
    }

    const int width = size().width();
    const int height = size().height();

    QRect iconRect(qMax(0, (width / 2) - (m_iconHeight / 2)), 0, m_iconHeight, m_iconHeight);
    painter->drawPixmap(iconRect, m_appletItem->icon().pixmap(m_iconHeight, m_iconHeight));

    if (m_appletItem->running() > 0) {
        QSize runningIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        painter->drawPixmap(iconRect.bottomLeft().x(), iconRect.bottomLeft().y() - runningIconSize.height(),
                            m_runningIcon.pixmap(runningIconSize));
    }

    QRectF textRect(0, iconRect.bottom() + 2, width, height - iconRect.bottom());
    painter->drawText(textRect, Qt::AlignTop | Qt::AlignCenter | Qt::TextWordWrap, m_appletItem->name());
 }
