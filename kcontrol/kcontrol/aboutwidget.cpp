/*

  Copyright (c) 1999 Matthias Hoelzer-Kluepfel <hoelzer@kde.org>
 
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



/* This is the default widget for kcc
   Author: Markus Wuebben
	   <markus.wuebben@kde.org>
   Date:   September '97         */


#include <unistd.h>
#include <sys/utsname.h>
#include <stdlib.h>


#include <qlayout.h>
#include <qlabel.h>


#include <kglobal.h>
#include <klocale.h>
#include <kiconloader.h>


#include "config.h"
#include "aboutwidget.h"
#include "aboutwidget.moc"


AboutWidget::AboutWidget(QWidget *parent , const char *name)
  : QWidget(parent, name)
{
  char buf[128];
  struct utsname info;

  setCaption(i18n("About"));

  QGridLayout *grid = new QGridLayout(this, 0, 0, 6, 6);
  
  QLabel *label = new QLabel(i18n("KDE Control Center"), this);
  label->setFont(QFont("times", 24, QFont::Bold, true));
  grid->addMultiCellWidget(label, 0,0, 0,2, AlignCenter);

  label = new QLabel(i18n("KDE Version:"), this);
  grid->addWidget(label, 2,0);
  label = new QLabel(VERSION, this);
  grid->addWidget(label, 2,2);

  label = new QLabel(i18n("User:"), this);
  grid->addWidget(label, 3,0);

  QString str;
  char *login = getlogin();
  if (!login)
    login = getenv("LOGNAME");
  if (!login)
    str = i18n("Unknown");
  else 
    str = login;

  label = new QLabel(str, this);
  grid->addWidget(label, 3,2);

  uname(&info);

  gethostname(buf, 128);
  label = new QLabel(i18n("Hostname:"), this);
  grid->addWidget(label, 4,0);
  label = new QLabel(buf, this);
  grid->addWidget(label, 4,2);
  
  label = new QLabel(i18n("System:"), this);
  grid->addWidget(label, 5,0);
  label = new QLabel(info.sysname, this);
  grid->addWidget(label, 5,2);
  
  label = new QLabel(i18n("Release:"), this);
  grid->addWidget(label, 6,0);
  label = new QLabel(info.release, this);
  grid->addWidget(label, 6,2);
  
  label = new QLabel(i18n("Version:"), this);
  grid->addWidget(label, 7,0);
  label = new QLabel(info.version, this);
  grid->addWidget(label, 7,2);
  
  label = new QLabel(i18n("Machine:"), this);
  grid->addWidget(label, 8,0);
  label = new QLabel(info.machine, this);
  grid->addWidget(label, 8,2);
    
  label = new QLabel(this);  
  label->setPixmap(KGlobal::iconLoader()->loadIcon("kdekcc"));
  grid->addMultiCellWidget(label, 9, 9, 0,2);

  grid->setColStretch(2, 1);
  grid->addColSpacing(1, 16);
  grid->setRowStretch(0, 10);
  grid->setRowStretch(1, 5);
  grid->setRowStretch(10,100);
}
