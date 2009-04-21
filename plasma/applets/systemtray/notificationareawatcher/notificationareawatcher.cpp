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

#include "notificationareawatcher.h"

#include <QDBusConnection>

#include <kglobal.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include "notificationareawatcheradaptor.h"
#include "notificationarea_interface.h"


static inline KAboutData aboutData()
{
    return KAboutData("notificationareawatcher", 0, ki18n("notificationareawatcher"), KDE_VERSION_STRING);
}

K_PLUGIN_FACTORY(NotificationAreaWatcherFactory,
                 registerPlugin<NotificationAreaWatcher>();
    )
K_EXPORT_PLUGIN(NotificationAreaWatcherFactory(aboutData()))

NotificationAreaWatcher::NotificationAreaWatcher(QObject *parent, const QList<QVariant>&)
      : KDEDModule(parent)
{
    setModuleName("NotificationAreaWatcher");
    new NotificationAreaWatcherAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.kde.NotificationAreaWatcher");
    dbus.registerObject("/NotificationAreaWatcher", this);
    m_dbusInterface = dbus.interface();

    connect(m_dbusInterface, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           this, SLOT(serviceChange(QString,QString,QString)));
}

NotificationAreaWatcher::~NotificationAreaWatcher()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService("org.kde.NotificationAreaWatcher");
}


void NotificationAreaWatcher::RegisterService(const QString &service)
{
    if (m_dbusInterface->isServiceRegistered(service).value() &&
        !m_registeredServices.contains(service)) {
        kDebug()<<"Registering"<<service<<"to system tray";

        //check if the service has registered a SystemTray object
        org::kde::NotificationAreaItem trayclient(service, "/NotificationAreaItem",
                                        QDBusConnection::sessionBus());
        if (trayclient.isValid()) {
            m_registeredServices.append(service);
            emit ServiceRegistered(service);
        }
    }
}

QStringList NotificationAreaWatcher::RegisteredServices() const
{
    return m_registeredServices;
}


void NotificationAreaWatcher::serviceChange(const QString& name,
                                const QString& oldOwner,
                                const QString& newOwner)
{
    //kDebug()<<"Service "<<name<<"status change, old owner:"<<oldOwner<<"new:"<<newOwner;

    if (newOwner.isEmpty()) {
        if (m_registeredServices.contains(name)) {
            m_registeredServices.removeAll(name);
            emit ServiceUnregistered(name);
        }

        if (m_notificationAreaServices.contains(name)) {
            m_notificationAreaServices.remove(name);
        }
    }
}

void NotificationAreaWatcher::RegisterNotificationArea(const QString &service)
{
    if (service.contains("org.kde.NotificationArea-") &&
        m_dbusInterface->isServiceRegistered(service).value() &&
        !m_notificationAreaServices.contains(service)) {
        kDebug()<<"Registering"<<service<<"as system tray";

        //check if the service has registered a SystemTray object
        org::kde::NotificationAreaItem tray(service, "/",
                                        QDBusConnection::sessionBus());

        if (tray.isValid()) {
            m_notificationAreaServices.insert(service);
        }
    }
}

bool NotificationAreaWatcher::IsNotificationAreaRegistered() const
{
    return !m_notificationAreaServices.isEmpty();
}

#include "notificationareawatcher.moc"
