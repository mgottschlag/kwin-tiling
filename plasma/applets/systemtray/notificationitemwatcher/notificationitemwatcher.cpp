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

#include "notificationitemwatcher.h"

#include <QDBusConnection>

#include <kglobal.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include "notificationitemwatcheradaptor.h"
#include "notificationitem_interface.h"


static inline KAboutData aboutData()
{
    return KAboutData("notificationitemwatcher", 0, ki18n("notificationitemwatcher"), KDE_VERSION_STRING);
}

K_PLUGIN_FACTORY(NotificationItemWatcherFactory,
                 registerPlugin<NotificationItemWatcher>();
    )
K_EXPORT_PLUGIN(NotificationItemWatcherFactory(aboutData()))

NotificationItemWatcher::NotificationItemWatcher(QObject *parent, const QList<QVariant>&)
      : KDEDModule(parent)
{
    setModuleName("NotificationItemWatcher");
    new NotificationItemWatcherAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.kde.NotificationItemWatcher");
    dbus.registerObject("/NotificationItemWatcher", this);
    m_dbusInterface = dbus.interface();

    connect(m_dbusInterface, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           this, SLOT(serviceChange(QString,QString,QString)));
}

NotificationItemWatcher::~NotificationItemWatcher()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService("org.kde.NotificationItemWatcher");
}


void NotificationItemWatcher::RegisterService(const QString &service)
{
    if (m_dbusInterface->isServiceRegistered(service).value() &&
        !m_registeredServices.contains(service)) {
        kDebug()<<"Registering"<<service<<"to system tray";

        //check if the service has registered a SystemTray object
        org::kde::NotificationItem trayclient(service, "/NotificationItem",
                                        QDBusConnection::sessionBus());
        if (trayclient.isValid()) {
            m_registeredServices.append(service);
            emit ServiceRegistered(service);
        }
    }
}

QStringList NotificationItemWatcher::RegisteredServices() const
{
    return m_registeredServices;
}


void NotificationItemWatcher::serviceChange(const QString& name,
                                const QString& oldOwner,
                                const QString& newOwner)
{
    Q_UNUSED(oldOwner)
    //kDebug()<<"Service "<<name<<"status change, old owner:"<<oldOwner<<"new:"<<newOwner;

    if (newOwner.isEmpty()) {
        if (m_registeredServices.contains(name)) {
            m_registeredServices.removeAll(name);
            emit ServiceUnregistered(name);
        }

        if (m_notificationHostServices.contains(name)) {
            m_notificationHostServices.remove(name);
        }
    }
}

void NotificationItemWatcher::RegisterNotificationHost(const QString &service)
{
    if (service.contains("org.kde.NotificationHost-") &&
        m_dbusInterface->isServiceRegistered(service).value() &&
        !m_notificationHostServices.contains(service)) {
        kDebug()<<"Registering"<<service<<"as system tray";

        //check if the service has registered a SystemTray object
        org::kde::NotificationItem tray(service, "/",
                                        QDBusConnection::sessionBus());

        if (tray.isValid()) {
            m_notificationHostServices.insert(service);
            emit NotificationHostRegistered();
        }
    }
}

bool NotificationItemWatcher::IsNotificationHostRegistered() const
{
    return !m_notificationHostServices.isEmpty();
}

int NotificationItemWatcher::ProtocolVersion() const
{
    return 0;
}

#include "notificationitemwatcher.moc"
