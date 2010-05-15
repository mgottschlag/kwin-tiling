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
    m_removeIcon("edit-delete"),
    m_stopIcon("media-playback-stop"),
    m_playIcon("media-playback-start"),
    m_removable(true),
    m_activity(new Activity(id, this))
{
    connect(this, SIGNAL(clicked(Plasma::AbstractIcon*)), m_activity, SLOT(activate()));
    connect(m_activity, SIGNAL(opened()), this, SLOT(repaint()));
    connect(m_activity, SIGNAL(closed()), this, SLOT(repaint()));
    connect(m_activity, SIGNAL(nameChanged(QString)), this, SLOT(setName(QString)));
    setName(m_activity->name());
}

ActivityIcon::~ActivityIcon()
{
}

QPixmap ActivityIcon::pixmap(const QSize &size)
{
    // we need the icon customizable
    // while the default one is an identicon
    return KIdenticonGenerator::self()->generate(size.width(), m_activity->id());
    //return m_icon.pixmap(size);
    //FIXME check whether Activity returns an icon, when that's implemented?
    //maybe move this code to Activity?
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
    qreal iconX = rect.x() + qMax(0.0, (rect.width() - iconSize()) / 2.0); //icon's centered

    if (m_removable) {
        //the Remove corner-button
        qreal removeX = iconX + iconSize() - cornerIconSize.width();
        qreal removeY = rect.y();
        painter->drawPixmap(removeX, removeY, m_removeIcon.pixmap(cornerIconSize));
    }

    if (m_activity->isRunning()) {
        //draw stop icon
        qreal stopX = iconX;
        qreal stopY = rect.y();
        painter->drawPixmap(stopX, stopY, m_stopIcon.pixmap(cornerIconSize));
    } else {
        //draw play icon, centered
        qreal playIconSize = KIconLoader::SizeMedium;
        qreal offset = (iconSize() - playIconSize) / 2.0;
        qreal playX = iconX + offset;
        qreal playY = rect.y() + offset;
        painter->drawPixmap(playX, playY, m_playIcon.pixmap(playIconSize));
    }
}

void ActivityIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    //check whether one of our corner icons was clicked
    //FIXME this is duplicate code, should get cleaned up later
    const QRectF rect = contentsRect();
    QSize cornerIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    qreal iconX = rect.x() + qMax(0.0, (rect.width() / 2) - (iconSize() / 2));

    if (m_removable) {
        qreal removeX = iconX + iconSize() - cornerIconSize.width();
        qreal removeY = rect.y();
        QRectF removeRect(QPointF(removeX, removeY), cornerIconSize);

        if (removeRect.contains(event->pos())) {
            m_activity->destroy();
            return;
        }
    }

    if (m_activity->isRunning()) {
        QRectF stopRect(QPointF(iconX, rect.y()), cornerIconSize);
        if (stopRect.contains(event->pos())) {
            m_activity->close();
            return;
        }
    }

    AbstractIcon::mouseReleaseEvent(event);
}

void ActivityIcon::repaint()
{
    update();
}

void ActivityIcon::setRemovable(bool removable)
{
    if (removable == m_removable) {
        return;
    }
    m_removable = removable;
    update();
}

