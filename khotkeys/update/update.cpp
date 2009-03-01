/****************************************************************************

 KHotKeys

 Copyright (C) 2003 Lubos Lunak <l.lunak@kde.org>

 Distributed under the terms of the GNU General Public License version 2.

****************************************************************************/

#define _UPDATE_CPP_

// #include <config-workspace.h>

#include "khotkeysiface.h"
#include "kglobalaccel.h"

#include <KDE/KAboutData>
#include <KDE/KApplication>
#include <KDE/KCmdLineArgs>
#include <KDE/KConfig>
#include <KDE/KStandardDirs>
#include <KDE/KDebug>

#include <QtDBus/QtDBus>

#include <settings.h>

using namespace KHotKeys;

int main( int argc, char* argv[] )
    {
    KAboutData aboutdata(
            "khotkeys_update",
            0,
            ki18n("KHotKeys Update Helper"),
            "4.2",
            ki18n("KHotKeys Update Helper"),
            KAboutData::License_GPL,
            ki18n("(C) 2003-2009 Lubos Lunak"));
    aboutdata.addAuthor(ki18n("Michael Jansen"),ki18n("Maintainer"),"kde@michael-jansen.biz");

    KCmdLineArgs::init( argc, argv, &aboutdata);

    KCmdLineOptions options;
    options.add("id <id>", ki18n("Id of the script to add to %1").subs(KHOTKEYS_CONFIG_FILE));
    KCmdLineArgs::addCmdLineOptions( options );

    KApplication app;
    KCmdLineArgs* args = KCmdLineArgs::parsedArgs();

    QString id = args->getOption( "id" );
    QString file = KStandardDirs::locate( "data", "khotkeys/" + id + ".khotkeys" );
    if( file.isEmpty())
        {
        kWarning() << "File " << id << " not found!" ;
        return 1;
        }
    //  KGlobalAccel::self()->overrideMainComponentData(KComponentData("khotkeys"));

    init_global_data( false, &app );
    Settings settings;
    // Do not include disabled gestures.
    settings.reread_settings(false);
    KConfig cfg(  file );
    if( !settings.import( cfg, false ))
        {
        kWarning() << "Import of " << id << " failed!" ;
        return 2;
        }
    settings.write_settings();
    QDBusConnection bus = QDBusConnection::sessionBus();
    if( bus.interface()->isServiceRegistered( "org.kde.kded" ))
        {
        org::kde::khotkeys iface("org.kde.kded", "/modules/khotkeys", bus);
        iface.reread_configuration();
        }
    return 0;
    }
