#include <shutdown.h>
#include <kcmdlineargs.h>
#include <kaboutdata.h>
#include <kapplication.h>

int
main(int argc, char *argv[])
{
   KAboutData about("kapptest", "kapptest", "version");
   KCmdLineArgs::init(argc, argv, &about);

   KApplication a;
   KSMShutdownFeedback::start();

   bool saveSession;
   KApplication::ShutdownType sdtype = KApplication::ShutdownTypeNone;
   KApplication::ShutdownMode sdmode = KApplication::ShutdownModeSchedule;
   (void)KSMShutdownDlg::confirmShutdown( saveSession, false, false,
                                          sdtype, sdmode );
   (void)KSMShutdownDlg::confirmShutdown( saveSession, true, false,
                                          sdtype, sdmode );
   (void)KSMShutdownDlg::confirmShutdown( saveSession, true, true,
                                          sdtype, sdmode );

   KSMShutdownFeedback::stop();
}
