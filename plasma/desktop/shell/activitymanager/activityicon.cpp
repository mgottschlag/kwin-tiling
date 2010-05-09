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

#include <QPainter>

#include <KIconLoader>
#include <KIcon>

ActivityIcon::ActivityIcon(const QString &id)
    :AbstractIcon(0),
    m_id(id),
    m_removeIcon("edit-delete"),
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

    //the Remove corner-button
    const QRectF rect = contentsRect();
    const int width = rect.width();
    QSize cornerIconSize(KIconLoader::SizeSmall, KIconLoader::SizeSmall);
    //FIXME these calculations are copy-pasted. need to replace them with something that I properly
    //understand after I get this drawing.
    QRect iconRect(rect.x() + qMax(0, (width / 2) - (iconSize() / 2)), rect.y(), iconSize(), iconSize());

    painter->drawPixmap(iconRect.bottomLeft().x(), iconRect.bottomLeft().y() - cornerIconSize.height(),
            m_removeIcon.pixmap(cornerIconSize));
    //TODO make the little icon actually clickable
    //TODO: play/stop, delete.
}


