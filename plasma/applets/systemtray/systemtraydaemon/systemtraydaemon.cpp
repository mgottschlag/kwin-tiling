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

#include "systemtraydaemon.h"

#include <QDBusConnection>

#include <kglobal.h>
#include <kaboutdata.h>
#include <kdebug.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include "systemtraydaemonadaptor.h"


static inline KAboutData aboutData()
{
    return KAboutData("systemtraydaemon", 0, ki18n("systemtraydaemon"), KDE_VERSION_STRING);
}

K_PLUGIN_FACTORY(SystemTrayDaemonFactory,
                 registerPlugin<SystemTrayDaemon>();
    )
K_EXPORT_PLUGIN(SystemTrayDaemonFactory(aboutData()))

SystemTrayDaemon::SystemTrayDaemon(QObject *parent, const QList<QVariant>&)
      : KDEDModule(parent)
{
    setModuleName("systemtraydaemon");
    new SystemtrayDaemonAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.registerService("org.kde.SystemTrayDaemon");
    dbus.registerObject("/SystemTrayWatcher", this);
    m_dbusInterface = dbus.interface();

    connect(m_dbusInterface, SIGNAL(serviceOwnerChanged(QString,QString,QString)),
           this, SLOT(serviceChange(QString,QString,QString)));
}

SystemTrayDaemon::~SystemTrayDaemon()
{
    QDBusConnection dbus = QDBusConnection::sessionBus();
    dbus.unregisterService("org.kde.SystemTrayDaemon");
}


void SystemTrayDaemon::registerService(const QString &service)
{
    if (m_dbusInterface->isServiceRegistered(service).value() &&
        !m_registeredServices.contains(service)) {
        kDebug()<<"Registering"<<service<<"to system tray";
        //TODO: add a check if the service has registered a SystemTray object?
        m_registeredServices.append(service);
        emit serviceRegistered(service);
    }
}

QStringList SystemTrayDaemon::registeredServices() const
{
    return m_registeredServices;
}


void SystemTrayDaemon::serviceChange(const QString& name,
                                const QString& oldOwner,
                                const QString& newOwner)
{
    kDebug()<<"Service "<<name<<"status change, old owner:"<<oldOwner<<"new:"<<newOwner;

    if (newOwner.isEmpty() && m_registeredServices.contains(name)) {
        m_registeredServices.removeAll(name);
        emit serviceUnregistered(name);
    }
}

#include "systemtraydaemon.moc"
