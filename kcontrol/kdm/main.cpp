/*
 * main.cpp
 *
 * Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 *
 * Requires the Qt widget libraries, available at no cost at
 * http://www.troll.no/
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
 */


#include <klocale.h>
#include <kglobal.h>


#include "kdm-appear.h"
#include "kdm-font.h"
#include "backgnd.h"
#include "kdm-users.h"
#include "kdm-sess.h"

#include "main.h"


KDModule::KDModule(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  tab = new QTabWidget(this);

  appearance = new KDMAppearanceWidget(this);
  tab->addTab(appearance, i18n("&Appearance"));
  connect(appearance, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  font = new KDMFontWidget(this);
  tab->addTab(font, i18n("&Font"));
  connect(font, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  background = new KBackground(this, "");
  tab->addTab(background, i18n("&Background"));
  connect(background, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  users = new KDMUsersWidget(this);
  tab->addTab(users, i18n("&Users"));
  connect(users, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  sessions = new KDMSessionsWidget(this,0);
  tab->addTab(sessions, i18n("&Sessions"));
  connect(sessions, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
}


QString KDModule::quickHelp() const
{
    return i18n(    "<h1>Login Manager</h1> In this module you can configure the "
                    "various aspects  of the KDE Login Manager.  This includes "
                    "the look and feel as well as the users that can be "
                    "selected for login. Note that you can only make changes "
                    "if you run the module with superuser rights. If you haven't started the KDE "
                    "Control Center with superuser rights (which is the completely right thing to "
                    "do, by the way), click on the <em>Run as root</em> button to acquire "
                    "superuser rights. You will be asked for the superuser password."
                    "<h2>Appearance</h2> On this tab page, you can figure how "
                    "the Login Manager should look like, which language it should use, and which "
                    "GUI style it should use. The language settings made here have no influence on "
                    "the user's language settings."
                    "<h2>Font</h2>Here you can choose the fonts that the Login Manager should use "
                    "for various purposes like greetings and user names. "
                    "<h2>Background</h2>If you want to set a special background for the login "
                    "screen, this is where to do it."
                    "<h2>Users</h2>On this tab page, you can select which users the Login Manager "
                    "will offer you for logging in."
                    "<h2>Sessions</h2> Here you can specify which types of sessions the Login "
                    "Manager should offer you for logging in. This includes the standard KDE "
                    "session as well as a classic fvwm session and a failsafe session." );
}


void KDModule::load()
{
  appearance->load();
  font->load();
  background->load();
  users->load();
  sessions->load();
}


void KDModule::save()
{
  appearance->save();
  font->save();
  background->save();
  users->save();
  sessions->save();
}


void KDModule::defaults()
{
  appearance->defaults();
  font->defaults();
  background->defaults();
  users->defaults();
  sessions->defaults();
}


void KDModule::moduleChanged(bool state)
{
  emit changed(state);
}


void KDModule::resizeEvent(QResizeEvent *)
{
  tab->setGeometry(0,0,width(),height());
}


extern "C"
{
  KCModule *create_kdm(QWidget *parent, const char *name)
  {
    KGlobal::locale()->insertCatalogue("kdmconfig");
    return new KDModule(parent, name);
  }
}


#include "main.moc"
