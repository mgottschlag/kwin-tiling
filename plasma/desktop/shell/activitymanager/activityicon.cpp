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

#include "activityicon.h"

#include "activity.h"
#include "desktopcorona.h"
#include "plasmaapp.h"

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
    m_removable(true)
{
    DesktopCorona *c = qobject_cast<DesktopCorona*>(PlasmaApp::self()->corona());
    m_activity = c->activity(id);
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
    return m_activity ? m_activity->pixmap(size) : QPixmap();
}

QMimeData* ActivityIcon::mimeData()
{
    //TODO: how shall we use d&d?
    return 0;
}

void ActivityIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractIcon::paint(painter, option, widget);

    if (!m_activity) {
        return;
    }

    const QRectF rect = contentsRect();
    QSize cornerIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    qreal iconX = rect.x() + qMax<double>(0.0, (rect.width() - iconSize()) / 2.0); //icon's centered

    if (m_activity->isRunning()) {
        if (m_removable) {
            //draw stop icon
            qreal stopX = iconX + iconSize() - cornerIconSize.width();
            qreal stopY = rect.y();
            painter->drawPixmap(stopX, stopY, m_stopIcon.pixmap(cornerIconSize));
        }
    } else {
        //draw play icon, centered
        qreal playIconSize = KIconLoader::SizeMedium;
        qreal offset = (iconSize() - playIconSize) / 2.0;
        qreal playX = iconX + offset;
        qreal playY = rect.y() + offset;
        painter->drawPixmap(playX, playY, m_playIcon.pixmap(playIconSize));

        if (m_removable) {
            //the Remove corner-button
            qreal removeX = iconX + iconSize() - cornerIconSize.width();
            qreal removeY = rect.y();
            painter->drawPixmap(removeX, removeY, m_removeIcon.pixmap(cornerIconSize));
        }
    }
}

void ActivityIcon::mouseReleaseEvent(QGraphicsSceneMouseEvent *event)
{
    if (!m_activity) {
        return;
    }

    //check whether one of our corner icons was clicked
    //FIXME this is duplicate code, should get cleaned up later
    const QRectF rect = contentsRect();
    QSize cornerIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    qreal iconX = rect.x() + qMax<double>(0.0, (rect.width() / 2) - (iconSize() / 2));

    qreal removeX = iconX + iconSize() - cornerIconSize.width();
    qreal removeY = rect.y();
    QRectF removeRect(QPointF(removeX, removeY), cornerIconSize);
    if (m_removable && removeRect.contains(event->pos())) {
        if (m_activity->isRunning()) {
            m_activity->close();
        } else {
            QTimer::singleShot(0, m_activity, SLOT(destroy()));
        }
        return;
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

Activity* ActivityIcon::activity()
{
    return m_activity;
}

void ActivityIcon::activityRemoved()
{
    m_activity = 0;
    deleteLater();
}

#include "activityicon.moc"

