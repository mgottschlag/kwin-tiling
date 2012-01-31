/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
 *   Copyright 2008 Chani Armitage <chanika@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as
 *   published by the Free Software Foundation; either version 2,
 *   or (at your option) any later version.
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
#include <KLocale>
#include <KIcon>

#include <config-workspace.h>
#include "plasmaapp.h"

static const char description[] = I18N_NOOP( "Plasma widgets over the screensaver" );
static const char version[] = "0.0";

//extern "C"
int main(int argc, char **argv)
{
    KAboutData aboutData("plasma-overlay",0 , ki18n("Plasma for the Screensaver"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("Copyright 2006-2008, The KDE Team"));
    aboutData.addAuthor(ki18n("Chani Armitage"),
                        ki18n("Author and maintainer"),
                        "chanika@gmail.com");
    aboutData.addAuthor(ki18n("Aaron J. Seigo"),
                        ki18n("Plasma Author and maintainer"),
                        "aseigo@kde.org");
    aboutData.addCredit(ki18n("John Lions"),
                        ki18n("In memory of his contributions, 1937-1998."),
                        0, "http://en.wikipedia.org/wiki/John_Lions");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("cheats",ki18n("Enables some cheats that are useful for debugging."));
    options.add("setup",ki18n("Start unlocked for configuration."));
    KCmdLineArgs::addCmdLineOptions(options);

    PlasmaApp *app = PlasmaApp::self();
    QApplication::setWindowIcon(KIcon("plasma"));
    KGlobal::locale()->insertCatalog(QLatin1String( "libkworkspace" ));
    KGlobal::locale()->insertCatalog(QLatin1String( "kscreenlocker_greet" ));
    app->disableSessionManagement(); // I assume we'd never want a screensaver thing reppearing on login?
    int rc = app->exec();
    delete app;
    return rc;
}
