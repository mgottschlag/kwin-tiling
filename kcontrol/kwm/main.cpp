/*
  main.cpp - kwm configuration

 * Copyright (c) 1997 Patrick Dowler dowler@morgul.fsh.uvic.ca
 * Copyright (c) 1998 Matthias Ettrich <ettrich@kde.org>

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

#include <qdir.h>
#include <kcontrol.h>
#include <kconfig.h>
#include <klocale.h>
#include "windows.h"
#include "desktop.h"
#include "titlebar.h"
#include "mouse.h"
#include "advanced.h"
#include <stdio.h>

#include <kwm.h>

KConfig *config;

class KKWMApplication : public KControlApplication
{
public:

  KKWMApplication(int &argc, char **arg, const char *name);

  void init();
  void apply();

private:

  KWindowConfig *options;
  KTitlebarButtons *buttons;
  KTitlebarAppearance *appearance;
  KDesktopConfig *desktop;
  KAdvancedConfig *advanced;
};


KKWMApplication::KKWMApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  buttons = 0; appearance = 0; desktop = 0; advanced = 0;

  ::config = new KConfig("kwmrc");

  if (runGUI())
    {
      if (!pages || pages->contains("buttons"))
	addPage(buttons = new KTitlebarButtons(dialog, "buttons"),
		i18n("&Buttons"), "kwm-2.html");
      if (!pages || pages->contains("titlebar"))
	addPage(appearance = new KTitlebarAppearance(dialog, "titlebar"),
		i18n("&Titlebar"), "kwm-3.html");
      if (!pages || pages->contains("borders"))
	addPage(desktop = new KDesktopConfig(dialog, "borders"),
		i18n("Bo&rders"), "kwm-4.html");

      if (!pages || pages->contains("advanced"))
	addPage(advanced = new KAdvancedConfig(dialog, "advanced"),
		i18n("&Advanced"), "kwm-6.html");

      if ( buttons || appearance || desktop || advanced )
        dialog->show();
      else
        {
          fprintf(stderr, i18n("usage: kcmkwm [-init | {buttons,titlebar,borders,advanced}]\n").ascii());
          justInit = TRUE;
        }

    }
}


void KKWMApplication::init()
{
}


void KKWMApplication::apply()
{
  if (desktop)
    desktop->applySettings();
  if (buttons)
    buttons->applySettings();
  if (appearance)
    appearance->applySettings();
  if (advanced)
    advanced->applySettings();

  KWM::configureWm();
}


int main(int argc, char **argv)
{
    KKWMApplication app(argc, argv, "kcmkwm");
    app.setTitle(i18n("Window manager style"));
    int result = 0;
    if (app.runGUI())
	result =  app.exec();
    delete config;
    return result;
}
