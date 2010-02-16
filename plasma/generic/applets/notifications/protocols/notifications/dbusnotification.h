/***************************************************************************
 *   dbusnotification.h                                                    *
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

#ifndef DBUSNOTIFICATION_H
#define DBUSNOTIFICATION_H

#include "../../core/notification.h"



class DBusNotification : public Notification
{
    Q_OBJECT

    friend class DBusNotificationProtocol;

public:
    DBusNotification(const QString &source, QObject *parent = 0);
    ~DBusNotification();

public slots:
    void remove();
    void triggerAction(const QString &actionId);

signals:
    void notificationDeleted(const QString &source);
    void actionTriggered(const QString &source, const QString &actionId);
    void unregisterNotification(const QString &source);

private:
    QString m_source;
};



#endif
