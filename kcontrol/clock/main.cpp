/*
 *  main.cpp
 *
 *  Copyright (C) 1998 Luca Montecchiani <m.luca@usa.net>
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
 *
 */
#include <unistd.h>

#include <qtabwidget.h>
#include <qlabel.h>
#include <qlayout.h>

#include <kglobal.h>
#include <klocale.h>
#include <kcmodule.h>

#include "main.h"
#include "main.moc"

#include "tzone.h"
#include "dtime.h"

KclockModule::KclockModule(QWidget *parent, const char *name)
  : KCModule(parent, name)
{ 
  QVBoxLayout *layout = new QVBoxLayout(this);
  tab = new QTabWidget(this);
  layout->addWidget(tab);
  
  dtime = new Dtime(this);
  tab->addTab(dtime, i18n("Date && Time"));
  connect(dtime, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  // TODO: Time Zone is badly broken right now
  // Just disable
  //  tzone = new Tzone(this);
  //  tab->addTab(tzone, i18n("Time Zone"));
  //  connect(tzone, SIGNAL(changed(bool)), this, SLOT(moduleChanged(bool)));

  if(getuid() != 0) {
    layout->addSpacing(5);
    layout->addWidget(new QLabel(i18n("You do not have authority to change system time"), this));
    layout->addWidget(new QLabel(i18n("To change time run Control Center as a root"), this));
    setButtons(Help|Cancel);
  }
  else
    setButtons(Help|Reset|Cancel|Apply|Ok);
}

void KclockModule::save()
{
  dtime->save();
  //  tzone->save();
}

void KclockModule::load()
{
  dtime->load();
  //  tzone->load();
}

QString KclockModule::quickHelp()
{
  return i18n("FIXME: add some help");
}

void KclockModule::moduleChanged(bool state)
{
  emit changed(state);
}

extern "C"
{
  
  KCModule *create_clock(QWidget *parent, const char *name) 
  { 
    KGlobal::locale()->insertCatalogue("kcmkclock");
    return new KclockModule(parent, name);
  }

}


