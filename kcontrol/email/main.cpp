/*
  main.cpp - An E-mail preferences configuration application

  Copyright (c) 1999 Preston Brown <pbrown@kde.org>
  
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


#include <kcontrol.h>
#include <klocale.h>

#include "email.h"

class KEmailApplication : public KControlApplication
{
public:
  KEmailApplication(int &argc, char **argv, const char *name);
  void apply();

private:
  KEmailConfig *email;
};

KEmailApplication::KEmailApplication(int &argc, char **argv, const char *name)
  : KControlApplication(argc, argv, name)
{
  email = 0L;

  if (runGUI()) {
    if (!pages || pages->contains("email"))
      addPage(email = new KEmailConfig(dialog, "email"),
	      i18n("E-mail"), "email-1.html");
    
    dialog->setApplyButton(i18n("&Apply"));
    dialog->setCancelButton(i18n("Cancel"));
    if (email)
      dialog->show();
    else {
      // just initialize
      justInit = TRUE;
    }
  }
}

void KEmailApplication::apply()
{
  if (email)
    email->applySettings();
}

int main(int argc, char **argv)
{
  KEmailApplication app(argc, argv, "kcmemail");
  app.setTitle(i18n("E-mail Configuration"));
 
  if (app.runGUI())	
    return app.exec();
  else
    return 0;
}
