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
 *  Foundation, Inc., 51 Franklin Steet, Fifth Floor, Boston, MA  02110-1301, USA.
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
#include <klocale.h>
#include <kmessagebox.h>
#include <kdialog.h>
#include <kio/netaccess.h>

//#include "xpm/world.xpm"
#include "tzone.h"
#include "tzone.moc"

#if defined(USE_SOLARIS)
#include <ktempfile.h>
#include <kstandarddirs.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>

#define ZONEINFODIR	"/usr/share/lib/zoneinfo"
#define INITFILE	"/etc/default/init"
#endif

Tzone::Tzone(QWidget * parent, const char *name)
  : QGroupBox(parent, name)
{
    QBoxLayout *top_lay = new QVBoxLayout( this, KDialog::spacingHint() );
    QBoxLayout *lay = new QHBoxLayout(top_lay);

    currentzonetitle = new QLabel(i18n("Current time zone: "), this);
    lay->addWidget(currentzonetitle);

    currentzone = new QLabel(this);
    lay->addWidget(currentzone);
    currentzone->setFrameStyle(QFrame::Sunken | QFrame::StyledPanel);

    QLabel* instructions = new QLabel(i18n("To change the time zone, select your area from the list below:"), this);
    top_lay->addWidget(instructions);

    tzonelist = new QComboBox( false, this, "ComboBox_1" );
    connect( tzonelist, SIGNAL(activated(int)), SLOT(handleZoneChange()) );
    top_lay->addWidget( tzonelist );

    fillTimeZones();

    load();

    tzonelist->setEnabled(getuid() == 0);
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

    fp = popen(buf, "r");
    if (fp)
    {
        while(fgets(buf, MAXPATHLEN - 1, fp) != NULL)
        {
            buf[strlen(buf) - 1] = '\0';
            list << i18n(buf);
            tzonenames << buf;
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
            if (fields.count() >= 3) {
                list << i18n(fields[2].utf8());
                tzonenames << fields[2];
            }
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
    return QString::fromLocal8Bit(result);
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

    fp = popen(buf, "r");
    if (fp)
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
      if (tzonelist->text(i) == i18n(sCurrentlySet.utf8()))
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
      // Find untranslated selected zone
      QStringList::Iterator it;
      for (it = tzonenames.begin(); it != tzonenames.end(); ++it)
        if (selectedzone == i18n((*it).utf8()))
          break;
      selectedzone = (*it);

#if defined(USE_SOLARIS)	// MARCO

        KTempFile tf( locateLocal( "tmp", "kde-tzone" ) );
        tf.setAutoDelete( true );
        QTextStream *ts = tf.textStream();

        QFile fTimezoneFile(INITFILE);
        bool updatedFile = false;

        if (tf.status() == 0 && fTimezoneFile.open(IO_ReadOnly))
        {
            bool found = false;
            
            QTextStream is(&fTimezoneFile);

            for (QString line = is.readLine(); !line.isNull();
                 line = is.readLine())
            {
                if (line.find("TZ=") == 0)
                {
                    *ts << "TZ=" << selectedzone << endl;
                    found = true;
                }
                else
                {
                    *ts << line << endl;
                }
            }

            if (!found)
            {
                *ts << "TZ=" << selectedzone << endl;
            }

            updatedFile = true;
            fTimezoneFile.close();
        }

        if (updatedFile)
        {
            ts->device()->reset();
            fTimezoneFile.remove();

            if (fTimezoneFile.open(IO_WriteOnly | IO_Truncate))
            {
                QTextStream os(&fTimezoneFile);

                for (QString line = ts->readLine(); !line.isNull();
                     line = ts->readLine())
                {
                    os << line << endl;
                }

                fchmod(fTimezoneFile.handle(),
                       S_IXUSR | S_IRUSR | S_IRGRP | S_IXGRP |
                       S_IROTH | S_IXOTH);
                fTimezoneFile.close();
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

        tz = "/usr/share/zoneinfo/" + selectedzone;

        kdDebug() << "Set time zone " << tz << endl;

	if (!QFile::remove("/etc/localtime"))
	{	
		//After the KDE 3.2 release, need to add an error message
	}
	else
		if (!KIO::NetAccess::file_copy(KURL(tz),KURL("/etc/localtime")))
			KMessageBox::error( 0,  i18n("Error setting new Time Zone."),
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
