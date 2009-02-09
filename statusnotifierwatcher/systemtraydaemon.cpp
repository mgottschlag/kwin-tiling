
#include "systemtraydaemon.h"

#include <QDBusConnection>

#include <kglobal.h>
#include <kaboutdata.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>
#include "systemtraydaemonadaptor.h"

static inline KAboutData aboutData()
{
    return KAboutData("systemtraydaemon", 0, ki18n("systemtraydaemon"), KDE_VERSION_STRING);
}


SystemTrayDaemon::SystemTrayDaemon(QObject *parent, const QList<QVariant>&)
      : KDEDModule(parent)
{
    new SystemtrayDaemonAdaptor(this);
    QDBusConnection dbus = QDBusConnection::sessionBus();

    dbus.registerObject("/SystemTrayDaemon", this);
}

SystemTrayDaemon::~SystemTrayDaemon()
{}


void SystemTrayDaemon::registerService(const QString &service)
{
    m_registeredServices.append(service);
    emit serviceRegistered(service);
}

QStringList SystemTrayDaemon::registeredServices() const
{
    return m_registeredServices;
}

K_PLUGIN_FACTORY(SystemTrayDaemonFactory,
                 registerPlugin<SystemTrayDaemon>();
    )
K_EXPORT_PLUGIN(SystemTrayDaemonFactory(aboutData()))

#include "systemtraydaemon.moc"
