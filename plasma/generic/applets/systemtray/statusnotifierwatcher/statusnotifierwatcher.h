/***************************************************************************
 *   Copyright 2009 by Marco Martin <notmart@gmail.com>                    *
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

#ifndef NOTIFICATIONAREAWATCHER
#define NOTIFICATIONAREAWATCHER

#include <kdedmodule.h>

#include <QObject>
#include <QStringList>
#include <QSet>

class QDBusConnectionInterface;

class StatusNotifierWatcher : public KDEDModule
{
    Q_OBJECT
    Q_PROPERTY(QStringList RegisteredStatusNotifierItems READ RegisteredStatusNotifierItems)
    Q_PROPERTY(bool HasStatusNotifierHostRegistered READ HasStatusNotifierHostRegistered)
    Q_PROPERTY(int ProtocolVersion READ ProtocolVersion)

public:
    StatusNotifierWatcher(QObject *parent, const QList<QVariant>&);
    ~StatusNotifierWatcher();

    QStringList RegisteredStatusNotifierItems() const;

    bool HasStatusNotifierHostRegistered() const;

    int ProtocolVersion() const;

public Q_SLOTS:
    void RegisterStatusNotifierItem(const QString &service);

    void RegisterStatusNotifierHost(const QString &service);

protected Q_SLOTS:
    void serviceChange(const QString& name,
                       const QString& oldOwner,
                       const QString& newOwner);

Q_SIGNALS:
    void ServiceRegistered(const QString &service);
    //TODO: decide if this makes sense, the systray itself could notice the vanishing of items, but looks complete putting it here
    void ServiceUnregistered(const QString &service);
    void StatusNotifierHostRegistered();

private:
    QDBusConnectionInterface *m_dbusInterface;
    QStringList m_registeredServices;
    QSet<QString> m_statusNotifierHostServices;
};
#endif
