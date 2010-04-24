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

#include <KIconLoader>
#include <KIcon>

ActivityIcon::ActivityIcon(const QString &id)
    :AbstractIcon(0),
    m_id(id),
    m_icon("plasma")
{
    //FIXME this may interfere with drag when we implement that
    //and should be on mouse *release*
    //and the whole selection thing in AbstractIcon seems a tad odd.
    connect(this, SIGNAL(selected(AbstractIcon*)), SLOT(activate()));
}

ActivityIcon::~ActivityIcon()
{
}

QPixmap ActivityIcon::pixmap(const QSize &size)
{
    return m_icon.pixmap(size);
}

QMimeData* ActivityIcon::mimeData()
{
    //TODO: how shall we use d&d?
    return 0;
}

void ActivityIcon::paint(QPainter *painter, const QStyleOptionGraphicsItem *option, QWidget *widget)
{
    AbstractIcon::paint(painter, option, widget);

    //TODO: play/stop, delete.
    //but first try to use Plasma::Icon.
}

void ActivityIcon::activate()
{
    emit activated(name());
}

