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
#include <KGlobalSettings>
#include <KLocale>
#include <KIcon>
#include <KDebug>

#include <QTime>

#include <config-workspace.h>
#include "plasmaapp.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#include <fixx11h.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

static const char description[] = I18N_NOOP( "The KDE desktop, panels and widgets workspace application." );
static const char version[] = "0.4";
QString plasmaLocale;

extern "C"
KDE_EXPORT int kdemain(int argc, char **argv)
{
    kDebug() << "!!{} STARTUP TIME" << QTime().msecsTo(QTime::currentTime()) << "START" << "(line:" << __LINE__ << ")";
  
    plasmaLocale = KLocale("libplasma").language();
    // dual head support
    int associatedScreen = 0;
#ifdef Q_WS_X11
    {
    if (KGlobalSettings::isMultiHead()) {
        Display *dpy = XOpenDisplay(NULL);
        if (!dpy) {
            fprintf(stderr, "%s: FATAL ERROR: couldn't open display %s\n",
                    argv[0], XDisplayName(NULL));
            exit(1);
        }

        int numberOfScreens = ScreenCount(dpy);
        associatedScreen = DefaultScreen(dpy);
        QString displayName = QString::fromLocal8Bit(XDisplayString(dpy));
        int pos = displayName.lastIndexOf('.');

        XCloseDisplay(dpy);
        dpy = 0;

        if (pos != -1) {
            displayName.truncate(pos);
        }

        if (numberOfScreens > 1) {
            for (int i = 0; i < numberOfScreens; ++i) {
                if (i != associatedScreen && fork() == 0) {
                    associatedScreen = i;
                    // break here because we are the child process, we don't
                    // want to fork() anymore
                    break;
                }
            }

            QString env = QString("DISPLAY=%2.%1").arg(associatedScreen).arg(displayName);

            if (putenv(strdup(env.toLocal8Bit()))) {
                fprintf(stderr,
                        "%s: WARNING: unable to set DISPLAY environment variable\n",
                        argv[0]);
                perror("putenv()");
            }
        }
    }
    }
#endif

    QByteArray appName = "plasma-desktop";
    if (associatedScreen > 0) {
        appName.append("-screen-").append(QByteArray::number(associatedScreen));
    }

    KAboutData aboutData(appName, 0, ki18n("Plasma Desktop Shell"),
                         version, ki18n(description), KAboutData::License_GPL,
                         ki18n("Copyright 2006-2009, The KDE Team"));
    aboutData.addAuthor(ki18n("Aaron J. Seigo"),
                        ki18n("Author and maintainer"),
                        "aseigo@kde.org");
    aboutData.addCredit(ki18n("John Lions"),
                        ki18n("In memory of his contributions, 1937-1998."),
                        0, "http://en.wikipedia.org/wiki/John_Lions");

    KCmdLineArgs::init(argc, argv, &aboutData);

    PlasmaApp *app = PlasmaApp::self();
    QApplication::setWindowIcon(KIcon("plasma"));
    app->disableSessionManagement(); // autostarted

    int rc = app->exec();    
    delete app;
    return rc;
}
