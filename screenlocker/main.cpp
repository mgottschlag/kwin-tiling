/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
#include <KDE/KAboutData>
#include <KDE/KCmdLineArgs>
#include <kdebug.h>
#include <KDE/KLocale>
#include <KDE/KGlobal>

#include "ksldapp.h"

static const char description[] = I18N_NOOP( "KDE Plasma Workspaces Screenlocker Daemon" );
static const char version[] = "3.0";

extern "C"
KDE_EXPORT int kdemain(int argc, char* argv[])
{
    KAboutData aboutData( "kscreenlocker", 0, ki18n( "Screen Locker" ),
                          version, ki18n(description), KAboutData::License_GPL,
                          ki18n("(c) 2011, Martin Gräßlin") );
    aboutData.addAuthor( ki18n("Martin Gräßlin"),
                         ki18n( "Author and maintainer" ),
                         "mgraesslin@kde.org" );
    aboutData.addAuthor( ki18n("Chani Armitage"),
                         ki18n("Author"),
                         "chanika@gmail.com");
    aboutData.addAuthor( ki18n("Oswald Buddenhagen"),
                         ki18n("Author"),
                         "ossi@kde.org");
    aboutData.addAuthor( ki18n("Chris Howells"),
                         ki18n("Author"),
                         "howells@kde.org");
    aboutData.addAuthor( ki18n("Luboš Luňák"),
                         ki18n("Author"),
                         "l.lunak@kde.org");
    aboutData.addAuthor( ki18n("Martin R. Jones"),
                         ki18n("Author"),
                         "mjones@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);
    KCmdLineOptions options;
    options.add("locked", ki18n("Starts the daemon in locked mode"));
    KCmdLineArgs::addCmdLineOptions(options);
    if (!KUniqueApplication::start()) {
        return 0;
    }

    ScreenLocker::KSldApp *app = ScreenLocker::KSldApp::self();
    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();
    if (args->isSet("locked")) {
        app->lock();
    }
    KGlobal::locale()->insertCatalog(QLatin1String( "libkworkspace" ));
    app->disableSessionManagement(); // autostarted
    int rc = app->exec();
    delete app;
    return rc;
}
