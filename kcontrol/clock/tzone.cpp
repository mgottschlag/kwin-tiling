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
#include <config.h>

#include <qlabel.h>
#include <qcombobox.h>
#include <qgroupbox.h>
#include <qlayout.h>
#include <qfile.h>
#include <qregexp.h>

#include <kdebug.h>
#include <kconfig.h>
#include <kglobal.h>
#include <klocale.h>
#include <kmessagebox.h>

#include "xpm/world.xpm"
#include "tzone.h"
#include "tzone.moc"

#if defined(USE_SOLARIS)
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ZONEINFODIR	"/usr/share/lib/zoneinfo"
#define INITFILE	"/etc/default/init"
#endif

Tzone::Tzone(QWidget * parent, const char *name)
  : QWidget (parent, name)
{
    QGroupBox* gBox = new QGroupBox ( this );

    QBoxLayout *top_lay = new QVBoxLayout( gBox, 10 );
    QBoxLayout *lay = new QHBoxLayout(top_lay);

    currentzonetitle = new QLabel(i18n("Current time zone: "), gBox);
    currentzonetitle->setAutoResize(true);
    lay->addWidget(currentzonetitle);

    currentzone = new QLabel(gBox);
    lay->addWidget(currentzone);
    currentzone->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);

    QLabel* instructions = new QLabel(i18n("To change the time zone, select your area from the list below:"), gBox);
    top_lay->addWidget(instructions);

    tzonelist = new QComboBox( FALSE, gBox, "ComboBox_1" );
    connect( tzonelist, SIGNAL(activated(int)), SLOT(handleZoneChange()) );
    top_lay->addWidget( tzonelist );

    QBoxLayout *top = new QVBoxLayout( this, 5 );
    top->addWidget(gBox);

    fillTimeZones();

    load();

    if (getuid() != 0)
        tzonelist->setEnabled(false);

}

void Tzone::fillTimeZones()
{
    QStringList	list;

    tzonelist->insertItem(i18n("[No selection]"));

#if defined(USE_SOLARIS)	// MARCO

    FILE *fp;
    char buf[MAXPATHLEN];
    
    snprintf(buf, MAXPATHLEN,
	    "/bin/find %s \\( -name src -prune \\) -o -type f -print | /bin/cut -b %d-",
	    ZONEINFODIR, strlen(ZONEINFODIR) + 2);
    
    if (fp = popen(buf, "r"))
      {
	while(fgets(buf, MAXPATHLEN - 1, fp) != NULL)
	  {
	    buf[strlen(buf) - 1] = '\0';
	    list << buf;
	  }
	pclose(fp);
      }
    
#else
    QFile f("/usr/share/zoneinfo/zone.tab");
    if (f.open(IO_ReadOnly))
    {
        QTextStream ts(&f);
        QRegExp spaces("[ \t]");
        for (QString line = ts.readLine(); !line.isNull(); line = ts.readLine())
        {
            if (line.isEmpty() || line[0] == '#')
                continue;
            QStringList fields = QStringList::split(spaces, line);
            if (fields.count() >= 3)
                list << fields[2];
        }
    }
#endif // !USE_SOLARIS

    list.sort();

    tzonelist->insertStringList(list);
}

QString Tzone::currentZone() const
{
    QCString result(100);
    time_t now = time(0);
    tzset();
    strftime(result.data(), result.size(), "%Z", localtime(&now));
    kdDebug() << "strftime returned: " << result << endl;
    return QString::fromLatin1(result);
}

void Tzone::load()
{
    QString sCurrentlySet(i18n("Unknown"));
    currentzone->setText(currentZone());

    // read the currently set time zone

#if defined(USE_SOLARIS)	// MARCO
    FILE *fp;
    char buf[MAXPATHLEN];

    snprintf(buf, MAXPATHLEN,
	     "/bin/fgrep 'TZ=' %s | /bin/head -n 1 | /bin/cut -b 4-",
	     INITFILE);
    
    if (fp = popen(buf, "r"))
      {
	if (fgets(buf, MAXPATHLEN - 1, fp) != NULL)
	  {
	    buf[strlen(buf) - 1] = '\0';
	    sCurrentlySet = QString(buf);
	  }
	pclose(fp);
      }
#else    
    QFile f("/etc/timezone");
    if(f.open(IO_ReadOnly))
    {
        QTextStream ts(&f);
        ts >> sCurrentlySet;
    }
#endif // !USE_SOLARIS

    // find the currently set time zone and select it
    for (int i = 0; i < tzonelist->count(); i++)
    {
        if (tzonelist->text(i) == sCurrentlySet)
        {
            tzonelist->setCurrentItem(i);
            break;
        }
    }
}

void Tzone::save()
{
    QString tz;
    QString selectedzone(tzonelist->currentText());
    QString currentlySetZone;

    if( selectedzone != i18n("[No selection]"))
    {

#if defined(USE_SOLARIS)	// MARCO
      char buf[MAXPATHLEN];
      
      strcpy(buf, "/tmp/kde-tzone-XXXXXX");
      mktemp(buf);
      
      if (*buf == '\0')
	{
				// Could not create a temporary file
				// name.
	  return;
	}

      QFile tf(buf);
      QFile fTimezoneFile(INITFILE);
      bool updatedFile = false;
      
      if (tf.open(IO_WriteOnly | IO_Truncate) &&
	  fTimezoneFile.open(IO_ReadOnly))
	{
	  bool found = false;
	  QTextStream ts(&tf);
	  QTextStream is(&fTimezoneFile);
	  
	  for (QString line = is.readLine(); !line.isNull();
	       line = is.readLine())
	    {
	      if (line.find("TZ=") == 0)
		{
		  ts << "TZ=" << selectedzone << endl;
		  found = true;
		}
	      else
		{
		  ts << line << endl;
		}
	    }
	  
	  if (!found)
	    {
	      ts << "TZ=" << selectedzone << endl;
	    }
	
	  updatedFile = true;
	  tf.close();
	  fTimezoneFile.close();
        }

      if (updatedFile)
	{
	  fTimezoneFile.remove();

	  if (tf.open(IO_ReadOnly) &&
	      fTimezoneFile.open(IO_WriteOnly | IO_Truncate))
	    {
	      QTextStream ts(&tf);
	      QTextStream os(&fTimezoneFile);

	      for (QString line = ts.readLine(); !line.isNull();
		   line = ts.readLine())
		{
		  os << line << endl;
		}

	      fchmod(fTimezoneFile.handle(),
		     S_IXUSR | S_IRUSR | S_IRGRP | S_IXGRP |
		     S_IROTH | S_IXOTH);
	      fTimezoneFile.close();
	      tf.remove();
	    }
	}


      QString val = selectedzone;
#else
        QFile fTimezoneFile("/etc/timezone");

        if (fTimezoneFile.open(IO_WriteOnly | IO_Truncate) )
        {
            QTextStream t(&fTimezoneFile);
            t << selectedzone;
            fTimezoneFile.close();
        }

        tz = "/usr/share/zoneinfo/" + tzonelist->currentText();

        kdDebug() << "Set time zone " << tz << endl;

        // This is extremely ugly. Who knows the better way?
        unlink( "/etc/localtime" );
        if (symlink( QFile::encodeName(tz), "/etc/localtime" ) != 0)
            KMessageBox::error( 0,  i18n("Error setting new Time Zone!"),
                                i18n("Timezone Error"));

        QString val = ":" + tz;
#endif // !USE_SOLARIS

        setenv("TZ", val.ascii(), 1);
        tzset();

    } else {
#if !defined(USE_SOLARIS) // Do not update the System!
        unlink( "/etc/timezone" );
        unlink( "/etc/localtime" );
		
        setenv("TZ", "", 1);
        tzset();
#endif // !USE SOLARIS		
    }
    
    currentzone->setText(currentZone());
}
