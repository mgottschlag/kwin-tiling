/*
 *  applettab.cpp
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

#include <qlayout.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>

#include "applettab.h"
#include "applettab.moc"


AppletTab::AppletTab( QWidget *parent, const char* name )
  : QWidget (parent, name)
{
  load();
}

void AppletTab::load()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("applets");

  delete c;
}

void AppletTab::save()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("applets");
  c->sync();

  delete c;
}

void AppletTab::defaults()
{
}

QString AppletTab::quickHelp()
{
  return i18n("");
}
