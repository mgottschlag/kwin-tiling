/*
 *   Copyright 2006-2007 Aaron Seigo <aseigo@kde.org>
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

#include <KApplication>
#include <KAboutData>
#include <KCmdLineArgs>
#include <KLocale>
#include <KIcon>

#include <config-workspace.h>
#include "plasmaapp.h"

static const char description[] = I18N_NOOP( "The KDE desktop, panels and widgets workspace application." );
static const char version[] = "0.1";

extern "C"
#ifdef Q_WS_WIN
int kdemain(int argc, char **argv)
#else
KDE_EXPORT int kdemain(int argc, char **argv)
#endif
{
    KAboutData aboutData("plasma", 0, ki18n("Plasma Workspace"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("Copyright 2006-2007, The KDE Team"));
    aboutData.addAuthor(ki18n("Aaron J. Seigo"),
                        ki18n("Author and maintainer"),
                        "aseigo@kde.org");
    aboutData.addCredit(ki18n("John Lions"),
                        ki18n("In memory of his contributions, 1937-1998."),
                        0, "http://en.wikipedia.org/wiki/John_Lions");

    KCmdLineArgs::init(argc, argv, &aboutData);

    KCmdLineOptions options;
    options.add("nodesktop",ki18n("Starts plasma as an ordinary window rather than as the desktop."));
    KCmdLineArgs::addCmdLineOptions(options);

    PlasmaApp *app = PlasmaApp::self();
    QApplication::setWindowIcon(KIcon("plasma"));
    app->disableSessionManagement(); // autostarted
    int rc = app->exec();
    delete app;
    return rc;
}
