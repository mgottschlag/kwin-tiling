/*
  main.cpp 

  written 1997-1999  by Mark Donohoe, Martin Jones, Matthias Hoelzer, Matthias Ettrich

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.

  */


#include <stdio.h>

#include <klocale.h>
#include <kcontrol.h>
#include <kimgio.h>
#include "display.h"
#include "general.h"
#include <qfont.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kprocess.h>
#include <dcopclient.h>
#include <X11/Xlib.h>


bool runResourceManager = FALSE;

class KDisplayApplication : public KControlApplication
{
public:

  KDisplayApplication(int &argc, char **arg, const char *name);

  void init();
  void apply();
  void defaultValues();
  void writeQDesktopProperties( QPalette pal, QFont font);

private:

  KGeneral *general;
};


KDisplayApplication::KDisplayApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  general = 0; 

  if (runGUI())
    {
      if (!pages || pages->contains("style"))
	addPage(general = new KGeneral(dialog, KDisplayModule::Setup),
		i18n("&Style"), "kdisplay-7.html");

      if (general)
        dialog->show();
      else
        {
          fprintf(stderr, i18n("usage: kcmdisplay [-init | {style]\n").ascii());
          justInit = TRUE;
        }

    }
}


void KDisplayApplication::init()
{
  KGeneral *general = new KGeneral(0, KDisplayModule::Init);
  delete general;

  KConfigGroupSaver saver(KGlobal::config(), "X11");
  if (KGlobal::config()->readBoolEntry( "useResourceManager", true )){
      KProcess proc;
      proc.setExecutable("krdb");
      proc.start( KProcess::Block );
  }


}


void KDisplayApplication::apply()
{
  if (general)
    general->applySettings();

  kapp->config()->sync();
  
  
  if ( runResourceManager ) {
      QApplication::setOverrideCursor( waitCursor );
      KProcess proc;
      proc.setExecutable("krdb");
      proc.start( KProcess::Block );
      QApplication::restoreOverrideCursor();
      runResourceManager = FALSE;
  }
}

void KDisplayApplication::defaultValues()
{
  if (general)
    general->defaultSettings();
}

void KDisplayApplication::writeQDesktopProperties( QPalette pal, QFont font)
{

    QByteArray properties;
    QDataStream d( properties, IO_WriteOnly );

    d << pal << font;

    Atom a = XInternAtom(qt_xdisplay(), "_QT_DESKTOP_PROPERTIES", FALSE );

    XChangeProperty(qt_xdisplay(),  qt_xrootwin(),
		    a, a, 8, PropModeReplace,
		    (unsigned char*) properties.data(), properties.size());
    QApplication::flushX();
}

int main(int argc, char **argv)
{
  //QApplication::setColorSpec( QApplication::ManyColor );
  // Please don't use this as it prevents users from choosing exactly the
  // colors they want - Mark Donohoe

  KDisplayApplication app(argc, argv, "kcmdisplay");
  app.setTitle(i18n("Display settings"));

  kimgioRegister();

  if (app.runGUI())
  {
    DCOPClient *client = app.dcopClient();
    client->attach();
    client->registerAs(app.name());
    int rv = app.exec();
    client->detach();
    return rv;
  }
  else
  {
    app.init();
    return 0;
  }
}




