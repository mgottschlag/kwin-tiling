#include <shutdown.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>
#include <kiconloader.h>

int
main(int argc, char *argv[])
{
   KAboutData about("kapptest", "kapptest", "version");
   KCmdLineArgs::init(argc, argv, &about);

   KApplication a;
   a.iconLoader()->addAppDir("ksmserver");
   KSMShutdownFeedback::start();

   KApplication::ShutdownType sdtype = KApplication::ShutdownTypeNone;
   (void)KSMShutdownDlg::confirmShutdown( true,
                                          sdtype );
/*   (void)KSMShutdownDlg::confirmShutdown( false,
                                          sdtype ); */

   KSMShutdownFeedback::stop();
}
