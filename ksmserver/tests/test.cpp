#include "shutdowndlg.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kworkspace/kworkspace.h>

int
main(int argc, char *argv[])
{
    KAboutData about("kapptest", 0, ki18n("kapptest"), "version");
    KCmdLineArgs::init(argc, argv, &about);

    KCmdLineOptions options;
    options.add("t");
    options.add("type <name>", ki18n("The type of shutdown to emulate: Default, None, Reboot, Halt or Logout"), "None");
    KCmdLineArgs::addCmdLineOptions(options);

    KCmdLineArgs *args = KCmdLineArgs::parsedArgs();

    KApplication a;
    KIconLoader::global()->addAppDir("ksmserver");
    KSMShutdownFeedback::start();

    QString sdtypeOption = args->getOption("type").toLower();

    KWorkSpace::ShutdownType sdtype = KWorkSpace::ShutdownTypeDefault;
    if (sdtypeOption == "reboot") {
        sdtype = KWorkSpace::ShutdownTypeReboot;
    } else if (sdtypeOption == "halt") {
        sdtype = KWorkSpace::ShutdownTypeHalt;
    } else if (sdtypeOption == "logout") {
        sdtype = KWorkSpace::ShutdownTypeNone;
    }

    QString bopt;
    (void)KSMShutdownDlg::confirmShutdown( true, true, sdtype, bopt, "default" );
/*   (void)KSMShutdownDlg::confirmShutdown( false, false, sdtype, bopt ); */

    KSMShutdownFeedback::stop();
}
