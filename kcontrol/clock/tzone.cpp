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
#include <time.h>

#include <qlabel.h>
#include <qmsgbox.h>
#include <qcombo.h>
#include <qpixmap.h>
#include <qlayout.h>
#include <qfile.h>
#include <qtextstream.h>

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

        currentzonetitle = new QLabel(this);
        currentzonetitle->setAutoResize(true);
        currentzonetitle->setText(i18n("Current time zone: "));
        currentzonetitle->move(10, 10);

        currentzone = new QLabel(this);
        currentzone->setAutoResize(true);
        currentzone->move(currentzonetitle->width() + 20, 10);

        QLabel* instructions = new QLabel(this);
        instructions->setAutoResize(true);
        instructions->setText(i18n("To change the timezone, select your area from the list below:"));
        instructions->move(10, 50);

  tzonelist = new QComboBox( FALSE, frame1, "ComboBox_1" );
  connect( tzonelist, SIGNAL(activated(int)), SLOT(zone_changed()) );
  tzonelist->setAutoResize( FALSE );
  v1->addWidget( tzonelist );

  QHBoxLayout *top = new QHBoxLayout( this, 5 );
  top->addWidget(frame1, 1);

  // *************************************************************
  // End Dialog
  // *************************************************************

  fillTimeZones();

  load();
}

void Tzone::fillTimeZones()
{
        FILE *f;
        char tempstring[101] = "Unknown";
        QStrList list;

        getCurrentZone(tempstring);
        currentzone->setText(tempstring);

        tzonelist->insertItem(i18n("[No selection]"));
        // Read all system time zones
        system("cat /usr/share/zoneinfo/zone.tab | grep -e  ^[^#] | cut -f 3 > /tmp/_tzone_.txt");
        if((f = fopen("/tmp/_tzone_.txt", "r")) != NULL)
        {
                while(fgets(tempstring, 100, f) != NULL)
                {
                        list.inSort(tempstring);
                }
                fclose(f);
        }
        remove("/tmp/_tzone_.txt");
        tzonelist->insertStrList(&list);
}

void Tzone::getCurrentZone(char* szZone)
{
        FILE *f;

        // Read the current time zone
        system("date +%Z > /tmp/_tzone_.txt");
        if((f = fopen("/tmp/_tzone_.txt", "r")) != NULL)
        {
                fscanf(f, "%s", szZone);
                fclose(f);
        }
        remove("/tmp/_tzone_.txt");
}

void Tzone::load()
{
  KConfig *config = KGlobal::config();
  config->setGroup("tzone");
        FILE *f;
        char tempstring[101] = "Unknown";
        char szCurrentlySet[101] = "Unknown";
        QStrList list;
        int nCurrentlySet = 0;

        getCurrentZone(tempstring);
        currentzone->setText(tempstring);

        // read the currently set time zone
        if((f = fopen("/etc/timezone", "r")) != NULL)
        {
                // get the currently set timezone
                fgets(szCurrentlySet, 100, f);
                fclose(f);
        }

        tzonelist->insertItem(i18n("[No selection]"));
        // Read all system time zones
        system("cat /usr/share/zoneinfo/zone.tab | grep -e  ^[^#] | cut -f 3 > /tmp/_tzone_.txt");
        if((f = fopen("/tmp/_tzone_.txt", "r")) != NULL)
        {
                while(fgets(tempstring, 100, f) != NULL)
                {
                        list.inSort(tempstring);
                }
                fclose(f);
        }
        remove("/tmp/_tzone_.txt");
        tzonelist->insertStrList(&list);

        // find the currently set time zone and select it
        for (int i = 0; i < tzonelist->count(); i++)
        {
                if (tzonelist->text(i) == QString::fromLatin1(szCurrentlySet))
                {
                        nCurrentlySet = i;
                        break;
                }
        }

        tzonelist->setCurrentItem(nCurrentlySet);
}

void Tzone::save()
{
	QString tz;
	QString selectedzone(tzonelist->currentText());
	QString currentlySetZone;

	if( selectedzone != i18n("[No selection]"))
	{
		QFile fTimezoneFile("/etc/timezone");

		if (fTimezoneFile.open(IO_WriteOnly | IO_Truncate) )
		{
			QTextStream t(&fTimezoneFile);
			t << selectedzone;
			fTimezoneFile.close();
		}
	}

  tz.sprintf("/usr/share/zoneinfo/%s", (tzonelist->currentText()).data());
  tz.truncate(tz.length()-1);

  kdDebug() << "Set time zone " << tz << endl;

  // This is extremely ugly. Who knows the better way?
  unlink( "/etc/localtime" );
  if (symlink( QFile::encodeName(tz), "/etc/localtime" ) != 0)
	{
		QMessageBox::warning( 0, i18n("Timezone Error"), i18n("Error setting new Time Zone!"));
	}
	
	QString val = ":" + tz;
	setenv("TZ", val.ascii(), 1);
	tzset();
  
	// write some stuff
  KConfig *config = KGlobal::config();
  config->setGroup("tzone");
  config->writeEntry("TZ", tzonelist->currentItem() );
  config->sync();
}

