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
#include <qpixmap.h>
#include <qlabel.h>

#include <kglobal.h>
#include <kstddirs.h>
#include <klocale.h>

#include "config.h"
#include "aboutwidget.h"
#include "aboutwidget.moc"


AboutWidget::AboutWidget(QWidget *parent , const char *name)
  : QWidget(parent, name)
{
  char buf[128];
  struct utsname info;
  QString str;
  QHBoxLayout *hbox1;
  QVBoxLayout *vbox1,*vbox2, *vbox3;

  setCaption(i18n("About"));

  QVBoxLayout *top = new QVBoxLayout(this,10,10);

  vbox3 = new QVBoxLayout(top);

  QLabel *label = new QLabel(i18n("KDE Control Center"), this);
  label->setFont(QFont("times", 24, QFont::Bold));
  label->setAlignment(AlignLeft);
  label->setMaximumHeight(40);
  vbox3->addWidget(label);

  label = new QLabel(i18n("The control center is a central place to configure your desktop environment. "
						  "Select a item from the index list on the right to load a configuration module. "
						  "Click on Desktop->Background for example to configure the desktop background.\n\n"
						  "If you are unsure about where to look for a configuration option you can "
						  "search a keyword list. "
						  "Enter a search string in the \"search\" tab on the right to start a query.\n\n"
						  "The \"help\" tab displays a short help text for the active control module."), this);
  label->setAlignment(AlignLeft | WordBreak);
  vbox3->addWidget(label);

  hbox1 = new QHBoxLayout(top);

  label = new QLabel(this);
  QPixmap pm(locate( "data", QString::fromLatin1("kdeui/pics/aboutkde.png")));
  label->setPixmap(pm);
  label->setFixedSize(pm.size());
  label->setFrameStyle(QFrame::Panel | QFrame::Sunken);
  label->setAlignment(AlignCenter);
  hbox1->addWidget( label );

  vbox1 = new QVBoxLayout(hbox1);
  hbox1->addSpacing(20);
  vbox2 = new QVBoxLayout(hbox1);

  vbox1->addWidget( new QLabel(i18n("KDE Version:"), this) );
  vbox2->addWidget( new QLabel(VERSION, this) );

  char *user = getlogin();
  if (!user) user = getenv("LOGNAME");
  if (!user) str = i18n("Unknown");  else str = user;
  vbox1->addWidget( new QLabel(i18n("User:"), this) );
  vbox2->addWidget( new QLabel(user, this) );

  gethostname(buf, 128);
  vbox1->addWidget( new QLabel(i18n("Hostname:"), this) );
  vbox2->addWidget( new QLabel(buf, this) );
  
  uname(&info);
  vbox1->addWidget( new QLabel(i18n("System:"), this) );
  vbox2->addWidget( new QLabel(info.sysname, this) );
  
  vbox1->addWidget( new QLabel(i18n("Release:"), this) );
  vbox2->addWidget( new QLabel(info.release, this) );
  
  vbox1->addWidget( new QLabel(i18n("Version:"), this) );
  vbox2->addWidget( new QLabel(info.version, this) );
  
  vbox1->addWidget( new QLabel(i18n("Machine:"), this) );
  vbox2->addWidget( new QLabel(info.machine, this) );
}
