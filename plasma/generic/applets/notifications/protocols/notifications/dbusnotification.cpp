/***************************************************************************
 *   dbusnotification.cpp                                                  *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "dbusnotification.h"

#include <KDebug>


DBusNotification::DBusNotification(const QString &source, QObject *parent)
    : Notification(parent),
      m_source(source)
{
}

DBusNotification::~DBusNotification()
{
    emit notificationDeleted(m_source);
}

void DBusNotification::remove()
{
    emit unregisterNotification(m_source);
    deleteLater();
}

void DBusNotification::triggerAction(const QString &actionId)
{
    emit actionTriggered(m_source, actionId);
}

