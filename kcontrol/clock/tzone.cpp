/*
 *  tzone.cpp
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>    

#include <qlabel.h>
#include <qmsgbox.h>
#include <qcombo.h>
#include <qpixmap.h> 
#include <qlayout.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kcmodule.h>

#include "xpm/world.xpm"
#include "tzone.h"
#include "tzone.moc"

Tzone::Tzone(QWidget * parent, const char *name)
  : KCModule (parent, name)
{
  // *************************************************************
  // Start Dialog
  // *************************************************************
  
  QFrame* frame1 = new QFrame( this );
  frame1->setFrameStyle( QFrame::Sunken | QFrame::Box );
  
  QVBoxLayout *v1 = new QVBoxLayout( frame1, 10 );
  
  tzonelist = new QComboBox( FALSE, frame1, "ComboBox_1" );
  connect( tzonelist, SIGNAL(activated(int)), SLOT(zone_changed()) );
  tzonelist->setAutoResize( FALSE );
  v1->addWidget( tzonelist );
  
  QLabel* worldMap = new QLabel( frame1, "WorldMap" );
  worldMap->setAlignment( QLabel::AlignCenter );
  v1->addWidget( worldMap );
  
  QHBoxLayout *top = new QHBoxLayout( this, 5 );
  top->addWidget(frame1, 1);
  
  // *************************************************************
  // End Dialog
  // *************************************************************
  
  fillTimeZones();
  
  QPixmap pm( world );
  worldMap->setPixmap(pm);
  
  load();
}

void Tzone::fillTimeZones()
{
  // Needs to be improved (Add zone names)
  for ( int i=-14 ; i <= 12 ; i++ )
    {
      QString s;
      s.sprintf( "GMT%+d", i);
      tzonelist->insertItem( s );
    }
}

void Tzone::load() 
{
  KConfig *config = KGlobal::config();
  config->setGroup("tzone");
  pos = config->readNumEntry("TZ", 14);  
  tzonelist->setCurrentItem(pos);
}

void Tzone::save()
{
  char tz[40];
  
  sprintf(tz, "/usr/share/zoneinfo/Etc/%s", (tzonelist->currentText()).data());
   
  kdDebug() << "Set time zone " << tz << endl;
  
  // This is extremely ugly. Who knows the better way?
  unlink( "/etc/localtime" );
  symlink( tz, "/etc/localtime" );
  
  // write some stuff 
  KConfig *config = KGlobal::config();
  config->setGroup("tzone");
  config->writeEntry("TZ", tzonelist->currentItem() );
  config->sync();
}

