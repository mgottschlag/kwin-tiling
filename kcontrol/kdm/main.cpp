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


#include "kdm-appear.h"
#include "kdm-font.h"
#include "backgnd.h"
#include "kdm-users.h"
#include "kdm-sess.h"
#include "kdm-lilo.h" 

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

  lilo = new KDMLiloWidget(this);
  tab->addTab(lilo, i18n("&Lilo"));
  connect(lilo, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
}


void KDModule::load()
{
  appearance->load();
  font->load();
  background->load();
  users->load();
  sessions->load();
  lilo->load();
}


void KDModule::save()
{
  appearance->save();
  font->save();
  background->save();
  users->save();
  sessions->save();
  lilo->save();
}


void KDModule::defaults()
{
  appearance->defaults();
  font->defaults();
  background->defaults();
  users->defaults();
  sessions->defaults();
  lilo->defaults();
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
    return new KDModule(parent, name);
  }
}


