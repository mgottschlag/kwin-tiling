
#include "systemtraydaemon.h"

#include <kglobal.h>
#include <kaboutdata.h>

#include <kpluginfactory.h>
#include <kpluginloader.h>


static inline KAboutData aboutData()
{
    return KAboutData("systemtraydaemon", 0, ki18n("systemtraydaemon"), KDE_VERSION_STRING);
}


SystemTrayDaemon::SystemTrayDaemon(QObject *parent, const QList<QVariant>&)
      : KDEDModule(parent)
{}

SystemTrayDaemon::~SystemTrayDaemon()
{}

K_PLUGIN_FACTORY(SystemTrayDaemonFactory,
                 registerPlugin<SystemTrayDaemon>();
    )
K_EXPORT_PLUGIN(SystemTrayDaemonFactory(aboutData()))

#include "systemtraydaemon.moc"
