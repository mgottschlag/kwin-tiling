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


#include "keyconfig.h"


#include "main.h"


KeyModule::KeyModule(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  tab = new QTabWidget(this);

  global = new KKeyModule(this, true);
  tab->addTab(global, i18n("&Global shortcuts"));
  connect(global, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  standard = new KKeyModule(this, false);
  tab->addTab(standard, i18n("&Application shortcuts"));
  connect(standard, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));
}


void KeyModule::load()
{
  global->load();
  standard->load();
}


void KeyModule::save()
{
  global->save();
  standard->save();
}


void KeyModule::defaults()
{
  global->defaults();
  standard->defaults();
}


void KeyModule::moduleChanged(bool state)
{
  emit changed(state);
}


void KeyModule::resizeEvent(QResizeEvent *)
{
  tab->setGeometry(0,0,width(),height());
}


extern "C"
{
  KCModule *create_keys(QWidget *parent, const char *name) 
  { 
    KGlobal::locale()->insertCatalogue("kcmkeys");
    return new KeyModule(parent, name);
  }
}


