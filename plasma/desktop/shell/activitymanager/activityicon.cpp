/*
 *   Copyright 2010 Chani Armitage <chani@kde.org>
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

#include "activity.h"
#include "activityicon.h"
#include "kidenticongenerator.h"

#include <QGraphicsSceneMouseEvent>
#include <QPainter>
#include <QCursor>

#include <KIconLoader>
#include <KIcon>

ActivityIcon::ActivityIcon(const QString &id)
    :AbstractIcon(0),
    m_id(id),
    m_removeIcon("edit-delete"),
    m_stopIcon("media-playback-stop"),
    m_activity(new Activity(id, this))
{
    //FIXME this may interfere with drag when we implement that
    //and should be on mouse *release*
    //and the whole selection thing in AbstractIcon seems a tad odd.
    connect(this, SIGNAL(selected(AbstractIcon*)), m_activity, SLOT(activate()));
}

ActivityIcon::~ActivityIcon()
{
}

QPixmap ActivityIcon::pixmap(const QSize &size)
{
    // we need the icon customizable
    // while the default one is an identicon
    return KIdenticonGenerator::self()->generate(size.width(), m_id);
    //return m_icon.pixmap(size);
    //FIXME use the activity thumbnail (once it's implemented).
    //no icons, no need to customize, just a little image of the *containment*
}

QMimeData* ActivityIcon::mimeData()
{
    //TODO: how shall we use d&d?
    return 0;
}

void ActivityIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractIcon::paint(painter, option, widget);

    const QRectF rect = contentsRect();
    QSize cornerIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    qreal iconX = rect.x() + qMax(0.0, (rect.width() / 2) - (iconSize() / 2)); //FIXME does the rounding error matter?

    //the Remove corner-button
    qreal removeX = iconX + iconSize() - cornerIconSize.width();
    qreal removeY = rect.y();
    painter->drawPixmap(removeX, removeY, m_removeIcon.pixmap(cornerIconSize));

    if (m_activity->isRunning()) {
        qreal stopX = iconX;
        qreal stopY = rect.y();
        painter->drawPixmap(stopX, stopY, m_stopIcon.pixmap(cornerIconSize));
    } else {
        //TODO draw play icon, centered
    }
}

void ActivityIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //check whether one of our corner icons was clicked
    //FIXME this is duplicate code, should get cleaned up later
    const QRectF rect = contentsRect();
    QSize cornerIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    qreal iconX = rect.x() + qMax(0.0, (rect.width() / 2) - (iconSize() / 2));
    qreal removeX = iconX + iconSize() - cornerIconSize.width();
    qreal removeY = rect.y();

    QRectF removeRect(QPointF(removeX, removeY), cornerIconSize);

    if (removeRect.contains(event->pos())) {
        setCursor(Qt::OpenHandCursor);
        m_activity->destroy();
        return;
    }

    if (m_activity->isRunning()) {
        QRectF stopRect(QPointF(iconX, rect.y()), cornerIconSize);
        if (stopRect.contains(event->pos())) {
            setCursor(Qt::OpenHandCursor);
            m_activity->close();
            update();
            return;
        }
    }

    AbstractIcon::mouseReleaseEvent(event);
}

