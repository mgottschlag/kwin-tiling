/*
  main.cpp - A sample KControl Application

  written 1997 by Matthias Hoelzer

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

#include <kslider.h>

#include <kcontrol.h>
#include <kimgio.h>
#include "display.h"
#include "colorscm.h"
#include "scrnsave.h"
#include "general.h"
#include "backgnd.h"
#include <qfont.h>
#include <kconfig.h>

class KDisplayApplication : public KControlApplication
{
public:

  KDisplayApplication(int &argc, char **arg, const char *name);

  void init();
  void apply();
  void defaultValues();
  void writeQDesktopProperties( QPalette pal, QFont font);

private:

  KColorScheme *colors;
  KScreenSaver *screensaver;
  KFonts *fonts;
  KGeneral *general;
  KBackground *background;
};


KDisplayApplication::KDisplayApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  colors = 0; screensaver = 0; fonts = 0; general = 0; background = 0;

  if (runGUI())
    {
      if (!pages || pages->contains("background"))
        addPage(background = new KBackground(dialog, KDisplayModule::Setup),
		i18n("&Background"), "kdisplay-3.html");
      if (!pages || pages->contains("screensaver"))
	addPage(screensaver = new KScreenSaver(dialog, KDisplayModule::Setup),
		i18n("&Screensaver"), "kdisplay-4.html");

      if (!pages || pages->contains("colors"))
	addPage(colors = new KColorScheme(dialog, KDisplayModule::Setup),
		i18n("&Colors"), "kdisplay-5.html");
      if (!pages || pages->contains("fonts"))
	addPage(fonts = new KFonts(dialog, KDisplayModule::Setup),
		i18n("&Fonts"), "kdisplay-6.html");
      if (!pages || pages->contains("style"))
	addPage(general = new KGeneral(dialog, KDisplayModule::Setup),
		i18n("&Style"), "kdisplay-7.html");

      if (background || screensaver || colors || fonts || general)
        dialog->show();
      else
        {
          fprintf(stderr, i18n("usage: kcmdisplay [-init | {background,screensaver,colors,fonts,style}]\n").ascii());
          justInit = TRUE;
        }

    }
}


void KDisplayApplication::init()
{
  KColorScheme *colors = new KColorScheme(0, KDisplayModule::Init);
  writeQDesktopProperties( colors->createPalette(), kapp->generalFont() );
  delete colors;
  KBackground *background =  new KBackground(0, KDisplayModule::Init);
  delete background;
  KScreenSaver *screensaver = new KScreenSaver(0, KDisplayModule::Init);
  delete screensaver;
  KFonts *fonts = new KFonts(0, KDisplayModule::Init);
  delete fonts;
  KGeneral *general = new KGeneral(0, KDisplayModule::Init);
  delete general;

  KConfigGroupSaver saver(kapp->getConfig(), "X11");
  if (kapp->getConfig()->readBoolEntry( "useResourceManager", true )){
      KProcess proc;
      proc.setExecutable("krdb");
      proc.start( KProcess::Block );
  }


}


void KDisplayApplication::apply()
{
  if (colors)
    colors->applySettings();
  if (background)
    background->applySettings();
  if (screensaver)
    screensaver->applySettings();
  if (fonts)
    fonts->applySettings();
  if (general)
    general->applySettings();
  
  if (colors || fonts) {
      QPalette pal = colors?colors->createPalette():*qApp->palette();
      
      KConfig *config = kapp->getConfig();
      config->reparseConfiguration();
      config->setGroup( "General" );
      QFont font = kapp->generalFont();
      font = config->readFontEntry( "font", &font);
      writeQDesktopProperties( pal, font);
  }
}

void KDisplayApplication::defaultValues()
{
  if (colors)
    colors->defaultSettings();
  if (background)
    background->defaultSettings();
  if (screensaver)
    screensaver->defaultSettings();
  if (fonts)
    fonts->defaultSettings();
  if (general)
    general->defaultSettings();
}

void KDisplayApplication::writeQDesktopProperties( QPalette pal, QFont font)
{
    
    QByteArray properties;
    QDataStream d( properties, IO_WriteOnly );

    d << pal << font;

    Atom a = XInternAtom(qt_xdisplay(), "QT_DESKTOP_PROPERTIES", FALSE );
    
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
    return app.exec();
  else
    {
      app.init();
      return 0;
    }
}




