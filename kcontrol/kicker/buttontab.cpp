/*
 *  buttontab.cpp
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
#include <qgroupbox.h>
#include <qwhatsthis.h>

#include <kconfig.h>
#include <kdialog.h>
#include <kglobal.h>
#include <klocale.h>

#include "buttontab.h"
#include "buttontab.moc"


ButtonTab::ButtonTab( QWidget *parent, const char* name )
  : QWidget (parent, name)
{
  layout = new QGridLayout(this, 3, 1,
                           KDialog::marginHint(),
                           KDialog::spacingHint());

  load();
}

void ButtonTab::load()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("buttons");

  delete c;
}

void ButtonTab::save()
{
  KConfig *c = new KConfig("kickerrc", false, false);
  
  c->setGroup("buttons");

  c->sync();

  delete c;
}

void ButtonTab::defaults()
{
}

QString ButtonTab::quickHelp()
{
  return i18n("");
}
