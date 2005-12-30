#include "desktop.h"

#include <kuniqueapplication.h>
#include <klocale.h>
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <kcrash.h>

#include <signal.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

static const char description[] =
        I18N_NOOP("The KDE desktop");

static const char version[] = "0.0.1";

static void signalHandler(int sigId)
{
    fprintf(stderr, "*** pdesktop got signal %d (Exiting)\n", sigId);
    KCrash::setEmergencySaveFunction(0); // No restarts any more
    // try to cleanup all windows
    signal(SIGTERM, SIG_DFL); // next one kills
    signal(SIGHUP,  SIG_DFL); // next one kills
    if (kapp)
        kapp->quit(); // turn catchable signals into clean shutdown
}

extern "C" KDE_EXPORT
int kdemain(int argc, char **argv)
{
    //setup signal handling
    signal(SIGTERM, signalHandler);
    signal(SIGHUP,  signalHandler);

    KAboutData aboutData("PlasmaDesktop", I18N_NOOP("PlasmaDesktop"),
                         version, description, KAboutData::License_LGPL,
                         "(c) 1998-2005, The KDesktop Authors");
    aboutData.addAuthor("David Faure", 0, "faure@kde.org");
    aboutData.addAuthor("Martin Koller", 0, "m.koller@surfeu.at");
    aboutData.addAuthor("Waldo Bastian", 0, "bastian@kde.org");
    aboutData.addAuthor("Luboš Lu�ák", 0, "l.lunak@kde.org");
    aboutData.addAuthor("Joseph Wenninger", 0, "kde@jowenn.at");
    aboutData.addAuthor("Tim Jansen", 0, "tim@tjansen.de");
    aboutData.addAuthor("Benoit Walter", 0, "b.walter@free.fr");
    aboutData.addAuthor("Torben Weis", 0, "weis@kde.org");
    aboutData.addAuthor("Matthias Ettrich", 0, "ettrich@kde.org");

    KCmdLineArgs::init(argc, argv, &aboutData);

    if (!KUniqueApplication::start()) {
        fprintf(stderr, "pdesktop is already running!\n");
        exit(0);
    }
    KUniqueApplication app;

    Plasma::Desktop desktop();

    return app.exec();
}
