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

#include <Plasma/Theme>

AppletIconWidget::AppletIconWidget(QGraphicsItem *parent, PlasmaAppletItem *appletItem, Plasma::FrameSvg *bgSvg)
    : QGraphicsWidget(parent),
      m_appletItem(appletItem),
      m_selectedBackgroundSvg(bgSvg),
      m_runningIcon("dialog-ok"),
      m_iconHeight(DEFAULT_ICON_SIZE),
      m_hovered(false),
      m_selected(false)
{
    setCacheMode(DeviceCoordinateCache);
    setFont(KGlobalSettings::smallestReadableFont());
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);
}

AppletIconWidget::~AppletIconWidget()
{
}

PlasmaAppletItem *AppletIconWidget::appletItem()
{
    return m_appletItem.data();
}

void AppletIconWidget::setIconSize(int height)
{
    m_iconHeight = height;

    QFontMetrics fm(font());
    const int minHeight = height + 2 + fm.height();
    qreal l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);
    setMinimumHeight(minHeight + t + b);

//    kDebug() << height << minimumHeight();
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
    if (m_appletItem && event->button() != Qt::LeftButton &&
        (event->pos() - event->buttonDownPos(Qt::LeftButton)).toPoint().manhattanLength() > QApplication::startDragDistance()) {
        event->accept();
        qDebug() << "Start Dragging";
        QDrag *drag = new QDrag(event->widget());
        QPixmap p = appletItem()->icon().pixmap(KIconLoader::SizeLarge, KIconLoader::SizeLarge);
        drag->setPixmap(p);

        QMimeData *data = m_appletItem.data()->mimeData();

        drag->setMimeData(data);
        drag->exec();

        mouseReleaseEvent(event);
        update(boundingRect());
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
    qreal l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);
    m_iconHeight = qBound(0, int(size().height() - fm.height() - t - b - 2), int(size().height()));
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
    update(boundingRect());
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

    PlasmaAppletItem *appletItem = m_appletItem.data();
    if (!appletItem) {
        return;
    }


    const QRectF rect = contentsRect();
    const int width = rect.width();
    const int height = rect.height();

    QRect iconRect(rect.x() + qMax(0, (width / 2) - (m_iconHeight / 2)), rect.y(), m_iconHeight, m_iconHeight);
    painter->drawPixmap(iconRect, appletItem->icon().pixmap(m_iconHeight, m_iconHeight));

    if (appletItem->running() > 0) {
        QSize runningIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
        painter->drawPixmap(iconRect.bottomLeft().x(), iconRect.bottomLeft().y() - runningIconSize.height(),
                            m_runningIcon.pixmap(runningIconSize));
    }

    QRectF textRect(rect.x(), iconRect.bottom() + 2, width, height - iconRect.height() - 2);
    painter->setPen(
        Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));
    painter->drawText(textRect, Qt::AlignTop | Qt::AlignCenter | Qt::TextWordWrap, appletItem->name());
}


