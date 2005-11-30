#include "shutdowndlg.h"
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kiconloader.h>
#include <kworkspace.h>

int
main(int argc, char *argv[])
{
   KAboutData about("kapptest", "kapptest", "version");
   KCmdLineArgs::init(argc, argv, &about);

   KApplication a;
   a.iconLoader()->addAppDir("ksmserver");
   KSMShutdownFeedback::start();

   KWorkSpace::ShutdownType sdtype = KWorkSpace::ShutdownTypeNone;
   QString bopt
   (void)KSMShutdownDlg::confirmShutdown( true,
                                          sdtype,
                                          bopt );
/*   (void)KSMShutdownDlg::confirmShutdown( false,
                                          sdtype,
                                          bopt ); */

   KSMShutdownFeedback::stop();
}
