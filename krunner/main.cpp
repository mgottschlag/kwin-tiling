/*
 *   Copyright (C) 2006 Aaron Seigo <aseigo@kde.org>
 *   Copyright (C) 2006 Zack Rusin <zack@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 as
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <KAboutData>
#include <KCmdLineArgs>
#include <kdebug.h>
#include <KLocale>
#include <KGlobal>

#include "krunnerapp.h"
#ifdef Q_WS_X11
#include "startupid.h"
#endif
#include "klaunchsettings.h" // contains startup config

#ifdef Q_WS_X11
#include <X11/extensions/Xrender.h>
#endif

static const char description[] = I18N_NOOP( "KDE run command interface" );
static const char version[] = "0.1";

extern "C"
KDE_EXPORT int kdemain(int argc, char* argv[])
{
#ifdef Q_WS_X11
    // krunner only works properly with Qt's native X11 backend; override any compile-time
    // or command line settings to raster or OpenGL.
    QApplication::setGraphicsSystem(QLatin1String( "native" ));
#endif

    KAboutData aboutData( "krunner", 0, ki18n( "Run Command Interface" ),
                          version, ki18n(description), KAboutData::License_GPL,
                          ki18n("(c) 2006, Aaron Seigo") );
    aboutData.addAuthor( ki18n("Aaron J. Seigo"),
                         ki18n( "Author and maintainer" ),
                         "aseigo@kde.org" );

    KCmdLineArgs::init(argc, argv, &aboutData);
    if (!KUniqueApplication::start()) {
        return 0;
    }

    KRunnerApp *app = KRunnerApp::self();
    KGlobal::locale()->insertCatalog(QLatin1String( "processui" ));
    KGlobal::locale()->insertCatalog(QLatin1String( "libplasma" ));
    app->disableSessionManagement(); // autostarted
    int rc = app->exec();
    delete app;
    return rc;
}

