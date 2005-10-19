
#include "kaccess.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>
#include <klocale.h>
extern "C" KDE_EXPORT int kdemain(int argc, char * argv[] )
{
  KAboutData about(I18N_NOOP("kaccess"), I18N_NOOP("KDE Accessibility Tool"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 2000, Matthias Hoelzer-Kluepfel"));

  about.addAuthor("Matthias Hoelzer-Kluepfel", I18N_NOOP("Author") , "hoelzer@kde.org");

  KCmdLineArgs::init( argc, argv, &about );

  if (!KAccessApp::start())
    return 0;

  KAccessApp app;
  app.disableSessionManagement();
  return app.exec();
}
