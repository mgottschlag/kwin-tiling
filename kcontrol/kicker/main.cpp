/*
 *  main.cpp
 *
 *  Copyright (c) 2000 Matthias Elter <elter@kde.org>
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
 */

#include <qtabwidget.h>
#include <qlayout.h>

#include <kapp.h>
#include <kconfig.h>
#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>
#include <kimageio.h>

#include <dcopclient.h>

#include "main.h"
#include "paneltab.h"
#include "menutab.h"

KickerConfig::KickerConfig(QWidget *parent, const char *name)
  : KCModule(parent, name)
{
  QVBoxLayout *layout = new QVBoxLayout(this);
  tab = new QTabWidget(this);
  layout->addWidget(tab);

  paneltab = new PanelTab(this);
  tab->addTab(paneltab, i18n("&Panel"));
  connect(paneltab, SIGNAL(changed()), this, SLOT(configChanged()));

  menutab = new MenuTab(this);
  tab->addTab(menutab, i18n("&Menus"));
  connect(menutab, SIGNAL(changed()), this, SLOT(configChanged()));

  load();
}

KickerConfig::~KickerConfig() {}

void KickerConfig::configChanged()
{
  emit changed(true);
}


void KickerConfig::load()
{
  paneltab->load();
  menutab->load();
  emit changed(false);
}

void KickerConfig::save()
{
  paneltab->save();
  menutab->save();
  emit changed(false);

  // Tell kicker about the new config file.
  if (!kapp->dcopClient()->isAttached())
    kapp->dcopClient()->attach();
  QByteArray data;
  kapp->dcopClient()->send( "kicker", "Panel", "configure()", data );
}

void KickerConfig::defaults()
{
  paneltab->defaults();
  menutab->defaults();
  emit changed(true);
}

extern "C"
{
  KCModule *create_kicker(QWidget *parent, const char *name)
  {
    KImageIO::registerFormats();
    KGlobal::locale()->insertCatalogue("kcmkicker");
    return new KickerConfig(parent, name);
  };
}
