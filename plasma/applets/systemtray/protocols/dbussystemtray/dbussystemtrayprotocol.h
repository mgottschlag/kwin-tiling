/***************************************************************************
 *   DBusSystemTrayProtocol.h                                            *
 *                                                                         *
 *   Copyright (C) 2008 Jason Stubbs <jasonbstubbs@gmail.com>              *
 *   Copyright (C) 2008 Dmitry Suzdalev <dimsuz@gmail.com>                 *
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

#ifndef DBUSSYSTEMTRAYPROTOCOL_H
#define DBUSSYSTEMTRAYPROTOCOL_H

#include "../../core/protocol.h"

#include "systemtraydaemon_interface.h"

#include <QHash>

#include <QDBusConnection>


namespace SystemTray
{

class DBusSystemTrayTask;

class DBusSystemTrayProtocol : public Protocol
{
    Q_OBJECT

public:
    DBusSystemTrayProtocol(QObject *parent);
    ~DBusSystemTrayProtocol();
    void init();

protected:
    void newTask(QString service);
    void cleanupTask(QString typeId);
    void initRegisteredServices();

protected Q_SLOTS:
    void serviceChange(const QString& name,
                       const QString& oldOwner,
                       const QString& newOwner);
    void registerWatcher(const QString& service);
    void unregisterWatcher(const QString& service);
    void serviceRegistered(const QString &service);
    void serviceUnregistered(const QString &service);

private:
    QDBusConnection m_dbus;
    QHash<QString, DBusSystemTrayTask*> m_tasks;
    org::kde::SystemtrayDaemon *m_sysTrayDaemon;
};

}


#endif
