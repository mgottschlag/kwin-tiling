/*
  This file is part of the KDE Display Manager Configuration package
  Copyright (C) 1997 Thomas Tanghus (tanghus@earthling.net)
  
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


#include "utils.h"

#include <qmessagebox.h>

#include "kdm-appear.h"
#include "kdm-font.h"
#include "kdm-bgnd.h"
#include "kdm-users.h"
#include "kdm-sess.h"
#include "kdm-lilo.h"
#include <kwm.h>
#include <klocale.h>
#include <kglobal.h>
#include <kstddirs.h>

class KDMConfigApplication : public KControlApplication
{
public:

  KDMConfigApplication(int &argc, char **arg, const char *name);

  void apply();

private:

  KDMAppearanceWidget *appearance;
  KDMFontWidget       *font;
  KDMBackgroundWidget *background;
  KDMUsersWidget      *users;
  KDMSessionsWidget   *sessions;
  KDMLiloWidget       *lilo;
  QStrList            *pages;
};


KDMConfigApplication::KDMConfigApplication(int &argc, char **argv, 
					   const char *name)
    : KControlApplication(argc, argv, name)
{
  appearance = 0;
  font = 0;
  background = 0;
  users = 0;
  sessions = 0;
  lilo = 0;

  pages = getPageList();

  if (runGUI())
  {
      kimgioRegister();
      KGlobal::dirs()->addResourceType("icon", KStandardDirs::kde_default("data") + "/kdm/pics/users");
      KGlobal::dirs()->addResourceType("icon", KStandardDirs::kde_default("data") + "/kdm/pics");

      if (!pages || pages->contains("appearance"))
	  addPage(appearance = new KDMAppearanceWidget(dialog, "appearance", FALSE),
		  i18n("&Appearance"), 
		  "kdm-appear.html");
      if (!pages || pages->contains("font"))
        addPage(font = new KDMFontWidget(dialog, "font", FALSE),
		i18n("&Fonts"),
		"kdm-font.html");
      if (!pages || pages->contains("background"))
	  addPage(background = new KDMBackgroundWidget(dialog, "background", FALSE),
		  i18n("&Background"), "kdm-backgnd.html");
      if (!pages || pages->contains("users"))
        addPage(users = new KDMUsersWidget(dialog, "users", FALSE),
                                  i18n("&Users"), "kdm-users.html");
      if (!pages || pages->contains("sessions"))
        addPage(sessions = new KDMSessionsWidget(dialog, "sessions", FALSE),
                                  i18n("&Sessions"), "kdm-sess.html");
      if (!pages || pages->contains("lilo"))
        addPage(lilo = new KDMLiloWidget(dialog, "lilo", FALSE),
                                  i18n("&Lilo"), "kdm-lilo.html");
      if (appearance || font || background || sessions || users || lilo)
        dialog->show();
      else
	  {
	      fprintf(stderr, i18n("usage: kdmconfig [-init | {appearance,font,background,sessions,users,lilo}]\n"));
	      justInit = TRUE;
	  }
      
  }
}

/*
void KDMConfigApplication::init()
{
  KDMConfigWidget *kdmconfig = new KDMConfigWidget(0, 0, TRUE);
  delete kdmconfig;
}
*/

void KDMConfigApplication::apply()
{
  //debug("KDMConfigApplication::apply()");
  QApplication::setOverrideCursor( waitCursor );

  if (appearance)
    appearance->applySettings();
  if (font)
    font->applySettings();
  if (background)
    background->applySettings();
  if (users)
    users->applySettings();
  if (sessions)
    sessions->applySettings();
  if (lilo)
    lilo->applySettings();

  QApplication::restoreOverrideCursor( );
}


int main(int argc, char **argv)
{
  KDMConfigApplication app(argc, argv, "kdmconfig");
  app.setTitle(i18n("KDM Configuration"));
  
  if (app.runGUI()) {
      
      QString file = locate("config", "kdmrc");
      if (file == KGlobal::dirs()->getSaveLocation("config") + "kdmrc") {
	  QString msg = i18n("You have a local config file %1  - \n"
			     "I will save your changes into this file.\n"
			     "If this isn't your intention, remove it please!").arg(file);
	  QMessageBox::warning( 0, i18n("KDM Setup - Wrong filename"), msg,
				i18n("&Ok"));
      }
      QFileInfo fi(file);
      if(fi.isReadable() && fi.isWritable())
	  return app.exec();
      else {
	  QString msg = i18n("Sorry, but you don't have read/write\n"
			     "permission to the KDM setup file %1.").arg(file);
	  QMessageBox::warning( 0, i18n("KDM Setup - Missing privileges"), msg,
				i18n("&Ok"));
      }
  }
  else
    {
//      app.init();
      return 0;
    }
}
