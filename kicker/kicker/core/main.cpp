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

#include <QString>

#include <config.h>
#include <klocale.h>
#include <kcmdlineargs.h>
#include <kdebug.h>
#include <kaboutdata.h>
#include <kconfig.h>
#include <dcopclient.h>
#include <dcopref.h>

#include "kicker.h"

int kicker_screen_number = 0;

static const char description[] =
        I18N_NOOP("The KDE panel");

static const char version[] = VERSION;

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
        if (multiHead.lower() == "true")
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

    KAboutData aboutData( appname.data(), I18N_NOOP("KDE Panel"),
                          version, description, KAboutData::License_BSD,
                          I18N_NOOP("(c) 1999-2004, The KDE Team") );

    aboutData.addAuthor("Aaron J. Seigo", I18N_NOOP("Current maintainer"), "aseigo@kde.org");
    aboutData.addAuthor("Matthias Elter",0, "elter@kde.org");
    aboutData.addAuthor("Matthias Ettrich",0, "ettrich@kde.org");
    aboutData.addAuthor("Wilco Greven",0, "greven@kde.org");
    aboutData.addAuthor("Rik Hemsley",0, "rik@kde.org");
    aboutData.addAuthor("Daniel M. Duley",0, "mosfet@kde.org");
    aboutData.addAuthor("Preston Brown",0, "pbrown@kde.org");
    aboutData.addAuthor("John Firebaugh",0, "jfirebaugh@kde.org");
    aboutData.addAuthor("Waldo Bastian", I18N_NOOP("Kiosk mode"), "bastian@kde.org");

    aboutData.addCredit("Jessica Hall", /* I18N_NOOP("KConfigXT") */ 0, "jes.hall@kdemail.net");
    aboutData.addCredit("Stefan Nikolaus", /* I18N_NOOP("Bug fixes") */ 0, "stefan.nikolaus@kdemail.net");
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

    // send it even before KApplication ctor, because ksmserver will launch another app as soon
    // as QApplication registers with it
    DCOPClient* cl = new DCOPClient;
    cl->attach();
    DCOPRef r("ksmserver", "ksmserver");
    r.setDCOPClient(cl);
    r.send("suspendStartup");
    delete cl;

    Kicker kicker;
    return kicker.exec();
}
