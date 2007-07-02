/*****************************************************************

Copyright (c) 1996-2002 the kicker authors. See file AUTHORS.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>

#include <X11/Xlib.h>
#include <fixx11h.h>

#include <QtDBus/QtDBus>



#include <kdeversion.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kconfig.h>

#include "kicker.h"

int kicker_screen_number = 0;

static const char description[] =
        I18N_NOOP("The KDE panel");

static void sighandler(int)
{
    fprintf(stderr, "kicker: sighandler called\n");
    QApplication::exit();
}

extern "C" KDE_EXPORT int kdemain( int argc, char ** argv )
{
    // TODO: Multihead should be utilizing the QDesktopWidget and QX11Info
    {
        QByteArray multiHead = getenv("KDE_MULTIHEAD");
        if (multiHead.toLower() == "true")
        {
            Display *dpy = XOpenDisplay(NULL);
            if (!dpy)
            {
                fprintf(stderr, "%s: FATAL ERROR: couldn't open display %s\n",
                        argv[0], XDisplayName(NULL));
                exit(1);
            }

            int number_of_screens = ScreenCount(dpy);
            kicker_screen_number = DefaultScreen(dpy);
            int pos;
            QByteArray display_name = XDisplayString(dpy);
            XCloseDisplay(dpy);
            dpy = 0;

            if ((pos = display_name.lastIndexOf('.')) != -1)
            {
                display_name.remove(pos, 10);
            }

            QString env;
            if (number_of_screens != 1)
            {
                for (int i = 0; i < number_of_screens; i++)
                {
                    if (i != kicker_screen_number && fork() == 0)
                    {
                        kicker_screen_number = i;
                        // break here because we are the child process, we don't
                        // want to fork() anymore
                        break;
                    }
                }

                env.sprintf("DISPLAY=%s.%d", display_name.data(), kicker_screen_number);

                if (putenv(strdup(env.toLocal8Bit().data())))
                {
                    fprintf(stderr,
                            "%s: WARNING: unable to set DISPLAY environment variable\n",
                            argv[0]);
                    perror("putenv()");
                }
            }
        }
    }

    QByteArray appname;
    if (kicker_screen_number == 0)
    {
        appname = "kicker";
    }
    else
    {
        appname = "kicker-screen-" + QByteArray::number(kicker_screen_number);
    }

    KAboutData aboutData( appname.data(), 0, ki18n("KDE Panel"),
                          KDE_VERSION_STRING, ki18n(description), KAboutData::License_BSD,
                          ki18n("(c) 1999-2004, The KDE Team") );

    aboutData.addAuthor(ki18n("Aaron J. Seigo"), ki18n("Current maintainer"), "aseigo@kde.org");
    aboutData.addAuthor(ki18n("Matthias Elter"),KLocalizedString(), "elter@kde.org");
    aboutData.addAuthor(ki18n("Matthias Ettrich"),KLocalizedString(), "ettrich@kde.org");
    aboutData.addAuthor(ki18n("Wilco Greven"),KLocalizedString(), "greven@kde.org");
    aboutData.addAuthor(ki18n("Rik Hemsley"),KLocalizedString(), "rik@kde.org");
    aboutData.addAuthor(ki18n("Daniel M. Duley"),KLocalizedString(), "mosfet@kde.org");
    aboutData.addAuthor(ki18n("Preston Brown"),KLocalizedString(), "pbrown@kde.org");
    aboutData.addAuthor(ki18n("John Firebaugh"),KLocalizedString(), "jfirebaugh@kde.org");
    aboutData.addAuthor(ki18n("Waldo Bastian"), ki18n("Kiosk mode"), "bastian@kde.org");

    aboutData.addCredit(ki18n("Jessica Hall"), /* ki18n("KConfigXT") */ KLocalizedString(), "jes.hall@kdemail.net");
    aboutData.addCredit(ki18n("Stefan Nikolaus"), /* ki18n("Bug fixes") */ KLocalizedString(), "stefan.nikolaus@kdemail.net");
    KCmdLineArgs::init(argc, argv, &aboutData);

    if (!Kicker::start())
    {
        kError() << "kicker is already running!" << endl;
        return 0;
    }

    if (signal(SIGTERM, sighandler) == SIG_IGN)
    {
        signal(SIGTERM, SIG_IGN);
    }
    if (signal(SIGINT, sighandler) == SIG_IGN)
    {
        signal(SIGINT, SIG_IGN);
    }
    if (signal(SIGHUP, sighandler) == SIG_IGN)
    {
        signal(SIGHUP, SIG_IGN);
    }

    {
        // send it even before KApplication ctor, because ksmserver will launch another app as soon
        // as QApplication registers with it
        QDBusInterface dbus("org.kde.ksmserver", "/ksmserver");
        dbus.call("suspendStartup", "kicker");
    }

    Kicker kicker;
    return kicker.exec();
}
