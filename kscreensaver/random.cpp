//-----------------------------------------------------------------------------
//
// Screen savers for KDE
//
// Copyright (c)  Martin R. Jones 1999
//
// This is an extremely simple program that starts a random screensaver.
//

#include <config.h>

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#include <qtextstream.h>

#include <kapp.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <kdesktopfile.h>
#include <krandomsequence.h>
#include <kdebug.h>

#include "vroot.h"

#define MAX_ARGS    20

void usage(char *name)
{
    printf(i18n("Usage: %1 [-setup] [args]\n").arg(name).local8Bit());
    printf(i18n("Starts a random screensaver.\n").local8Bit());
    printf(i18n("Any arguments (except -setup) are passed on to the screensaver.\n").local8Bit());
}

int main(int argc, char *argv[])
{
    KApplication app(argc, argv, "random");
    long windowId = 0;
    bool root = false;
    int i;
    char *sargs[MAX_ARGS];

    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-setup"))
        {
            kdDebug() << "-setup not yet implemented" << endl;
            exit(1);
        }
        else if (!strcmp(argv[i], "-help"))
        {
            usage(argv[0]);
            exit(1);
        }
        else if (!strcmp(argv[i], "-window-id"))
        {
            windowId = atol(argv[++i]);
        }
        else if (!strcmp(argv[i], "-root"))
        {
            root = true;
        }
    }

    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     "apps/ScreenSavers/");
    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     "System/ScreenSavers/");
    QStringList saverFileList = KGlobal::dirs()->findAllResources("scrsav",
                                                   "*.desktop", false, true);

    KRandomSequence rnd;
    int indx = rnd.getLong(saverFileList.count());
    QString filename = *(saverFileList.at(indx));

    KDesktopFile config(filename, true);

    QString cmd;
    if (windowId && config.hasActionGroup("InWindow"))
    {
        config.setActionGroup("InWindow");
    }
    else if (root && config.hasActionGroup("Root"))
    {
        config.setActionGroup("Root");
    }
    cmd = config.readEntry("Exec");

    QTextStream ts(&cmd, IO_ReadOnly);
    QString word;
    ts >> word;
    QString exeFile = KStandardDirs::findExe(word);

    if (!exeFile.isEmpty())
    {
        sargs[0] = new char [strlen(word.ascii())+1];
        strcpy(sargs[0], word.ascii());

        i = 1;
        while (!ts.atEnd() && i < MAX_ARGS-1)
        {
            ts >> word;
            if (word == "%w")
            {
                word = word.setNum(windowId);
            }

            sargs[i] = new char [strlen(word.ascii())+1];
            strcpy(sargs[i], word.ascii());
            i++;
        }

        sargs[i] = 0;

        execv(exeFile.ascii(), sargs);
    }

    // If we end up here then we couldn't start a saver.
    // If we have been supplied a window id or root window then blank it.
    if (windowId || root)
    {
      Window win = windowId ? windowId : RootWindow(qt_xdisplay(), qt_xscreen());
      XSetWindowBackground(qt_xdisplay(), win,
                          BlackPixel(qt_xdisplay(), qt_xscreen()));
      XClearWindow(qt_xdisplay(), win);
    }
}

