//-----------------------------------------------------------------------------
//
// Screen savers for KDE
//
// Copyright (c)  Martin R. Jones 1999
//
// This is an extremely simple program that starts a random screensaver.
//

#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <qtextstream.h>
#include <kapp.h>
#include <kstddirs.h>
#include <kglobal.h>
#include <klocale.h>
#include <ksimpleconfig.h>

#define MAX_ARGS    20

void usage(char *name)
{
    printf(i18n("Usage: %1 [-setup] [args]\n").arg(name));
    printf(i18n("Starts a random screensaver.\n"));
    printf(i18n("Any arguments (except -setup) are passed on to the screensaver.\n"));
}

int main(int argc, char *argv[])
{
    KApplication app(argc, argv);
    long windowId = 0;
    int i;
    char *sargs[MAX_ARGS];

    srandom(time(0));
    
    for (i = 1; i < argc; i++)
    {
        if (!strcmp(argv[i], "-setup"))
        {
            debug("-setup not yet implemented");
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
    }

    KGlobal::dirs()->addResourceType("scrsav",
                                     KGlobal::dirs()->kde_default("apps") +
                                     "ScreenSavers/");
    QStringList saverFileList = KGlobal::dirs()->findAllResources("scrsav",
                                                                  "*.desktop");

    int indx = random()%saverFileList.count();
    QString filename = *(saverFileList.at(indx));

    KSimpleConfig config(filename, true);
    config.setDesktopGroup();

    QString cmd;
    if (windowId)
    {
        cmd = config.readEntry("Exec-kss");
    }
    else
    {
        cmd = config.readEntry("Exec");
    }

    QTextStream ts(&cmd, IO_ReadOnly);
    QString word;
    ts >> word;
    QString exeFile = locate("exe", word);

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

    if (!exeFile.isEmpty())
    {
        execv(exeFile.ascii(), sargs);
    }
}

