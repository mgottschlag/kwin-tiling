#include <klocale.h>

#include "kaccess.h"
#include <kaboutdata.h>
#include <kcmdlineargs.h>

int main(int argc, char * argv[] )
{
  KAboutData about(I18N_NOOP("kaccess"), I18N_NOOP("KDE Accessibility Tool"),
                  0, 0, KAboutData::License_GPL,
                  I18N_NOOP("(c) 2000, Matthias Hoelzer-Kluepfel"));

  about.addAuthor("Matthias Hoelzer-Kluepfel", I18N_NOOP("Author") , "hoelzer@kde.org");

  KCmdLineArgs::init( argc, argv, &about );

  if (!KAccessApp::start())
    return 0;

  KAccessApp app;
  return app.exec();
}
