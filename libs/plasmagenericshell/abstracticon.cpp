/*
 *   Copyright (C) 2009 by Ana Cecília Martins <anaceciliamb@gmail.com>
 *   Copyright (C) 2010 by Chani Armitage <chani@kde.org>
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

#include "abstracticon.h"

#include <QApplication>
#include <QCursor>
#include <QFontMetrics>
#include <QGraphicsSceneMouseEvent>
#include <QPainter>

#include <KIconLoader>
#include <KIcon>
#include <KGlobalSettings>

#include <Plasma/Theme>

namespace Plasma
{

AbstractIcon::AbstractIcon(QGraphicsItem *parent)
    : QGraphicsWidget(parent),
      m_iconHeight(DEFAULT_ICON_SIZE),
      m_maxSize(maximumSize()),
      m_selected(false),
      m_hovered(false)
{
    setCacheMode(DeviceCoordinateCache);
    setFont(KGlobalSettings::smallestReadableFont());
    setAcceptHoverEvents(true);
    setCursor(Qt::OpenHandCursor);
}

AbstractIcon::~AbstractIcon()
{
}

void AbstractIcon::setIconSize(int height)
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

int AbstractIcon::iconSize() const
{
    return m_iconHeight;
}

void AbstractIcon::setName(const QString &name)
{
   m_name = name;
   update();
}

QString AbstractIcon::name() const
{
    return m_name;
}

void AbstractIcon::collapse()
{
    if (isVisible()) {
        setVisible(false);
        m_maxSize = maximumSize();
        kDebug() << m_maxSize;
        setMaximumSize(0, 0);
    }
}

void AbstractIcon::expand()
{
    if (! isVisible()) {
        setVisible(true);
        setMaximumSize(m_maxSize);
    }
}

void AbstractIcon::hoverEnterEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = true;
    emit(hoverEnter(this));
}

void AbstractIcon::hoverLeaveEvent(QGraphicsSceneHoverEvent *)
{
    m_hovered = false;
    emit(hoverLeave(this));
}

void AbstractIcon::mouseMoveEvent(QGraphicsSceneMouseEvent *event)
{
    if (event->button() != Qt::LeftButton &&
        (event->pos() - event->buttonDownPos(Qt::LeftButton)).toPoint().manhattanLength() > QApplication::startDragDistance()) {
        event->accept();
        qDebug() << "Start Dragging";
        QDrag *drag = new QDrag(event->widget());
        QPixmap p = pixmap(QSize(KIconLoader::SizeLarge, KIconLoader::SizeLarge));
        drag->setPixmap(p);

        drag->setMimeData(mimeData());
        drag->exec();

        mouseReleaseEvent(event);
        update(boundingRect());
    }
}

void AbstractIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    QGraphicsWidget::mouseReleaseEvent(event);
    setCursor(Qt::OpenHandCursor);
}

void AbstractIcon::resizeEvent(QGraphicsSceneResizeEvent *)
{
    QFontMetrics fm(font());
    qreal l, t, r, b;
    getContentsMargins(&l, &t, &r, &b);
    m_iconHeight = qBound(0, int(size().height() - fm.height() - t - b - 2), int(size().height()));
}

void AbstractIcon::mousePressEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    setCursor(Qt::ClosedHandCursor);
    emit(selected(this));
}

void AbstractIcon::mouseDoubleClickEvent(QGraphicsSceneMouseEvent *event)
{
    Q_UNUSED(event)
    update(boundingRect());
    emit(doubleClicked(this));
}

void AbstractIcon::setSelected(bool selected)
{
    m_selected = selected;
    update(0,0,boundingRect().width(), boundingRect().height());
}

void AbstractIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    Q_UNUSED(option)
    Q_UNUSED(widget)

    const QRectF rect = contentsRect();
    const int width = rect.width();
    const int height = rect.height();

    QRect iconRect(rect.x() + qMax(0, (width / 2) - (m_iconHeight / 2)), rect.y(), m_iconHeight, m_iconHeight);
    painter->drawPixmap(iconRect, pixmap(QSize(m_iconHeight, m_iconHeight)));

    QRectF textRect(rect.x(), iconRect.bottom() + 2, width, height - iconRect.height() - 2);
    painter->setPen(Plasma::Theme::defaultTheme()->color(Plasma::Theme::TextColor));

    int flags = Qt::AlignTop;// | Qt::TextWordWrap;
    QFontMetrics fm(font());
    if (fm.width(m_name) < textRect.width()) {
        flags |= Qt::AlignCenter;
    }
    painter->drawText(textRect, flags, m_name);
}

} // namespace Plasma


