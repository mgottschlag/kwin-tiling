
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

  // verify the Xlib has matching XKB extension
  int major = XkbMajorVersion;
  int minor = XkbMinorVersion;
  if (!XkbLibraryVersion(&major, &minor))
    {
      kError() << "Xlib XKB extension does not match" << endl;
      return 1;
    }
  kDebug() << "Xlib XKB extension major=" << major << " minor=" << minor << endl;

  // we need an application object for QX11Info
  KAccessApp app;

  // verify the X server has matching XKB extension
  // if yes, the XKB extension is initialized
  int opcode_rtrn;
  int error_rtrn;
  int xkb_opcode;
  if (!XkbQueryExtension(QX11Info::display(), &opcode_rtrn, &xkb_opcode, &error_rtrn,
			 &major, &minor))
    {
      kError() << "X server has not matching XKB extension" << endl;
      return 1;
    }
  kDebug() << "X server XKB extension major=" << major << " minor=" << minor << endl;

  app.setXkbOpcode(xkb_opcode);
  app.disableSessionManagement();
  return app.exec();
}
