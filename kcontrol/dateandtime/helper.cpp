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
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/*

 A helper that's run using kdesu and does the system modifications.

 TODO: The time is offset by the delay caused by entering kdesu password,
       fix it by passing current time and adjust.

*/

#include "helper.h"

#include <config-workspace.h>

#include <time.h>
#include <unistd.h>

#include <kconfig.h>
#include <kconfiggroup.h>
#include <kdebug.h>
#include <kstandarddirs.h>
#include <kprocess.h>
#include <qfile.h>

#if defined(USE_SOLARIS)
#include <ktemporaryfile.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/stat.h>
#endif

static int Dtime_save_ntp( const QStringList& ntpServers, bool ntpEnabled )
{
  int ret = 0;
  // write to the system config file
  KConfig _config( KDE_CONFDIR "kcmclockrc", KConfig::SimpleConfig);
  KConfigGroup config(&_config, "NTP");
  config.writeEntry("servers", ntpServers );
  config.writeEntry("enabled", ntpEnabled );

  QString ntpUtility;
  if(!KStandardDirs::findExe("ntpdate").isEmpty()) {
    ntpUtility = "ntpdate";
  } else if(!KStandardDirs::findExe("rdate").isEmpty()) {
    ntpUtility = "rdate";
  }

  if(ntpEnabled && !ntpUtility.isEmpty()){
    // NTP Time setting
    QString timeServer = ntpServers.first();
    if( timeServer.indexOf( QRegExp(".*\\(.*\\)$") ) != -1 ) {
      timeServer.replace( QRegExp(".*\\("), "" );
      timeServer.replace( QRegExp("\\).*"), "" );
      // Would this be better?: s/^.*\(([^)]*)\).*$/\1/
    }
    KProcess proc;
    proc << ntpUtility << timeServer;
    if( proc.execute() != 0 ){
      ret |= ERROR_DTIME_NTP;
    }
  } else {
    ret |= ERROR_DTIME_NTP;
  }
  return ret;
}

static int Dtime_save_date( const QString& date )
{
    int ret = 0;
    // User time setting
    KProcess c_proc;
    c_proc << "date" << date;
    int result = c_proc.execute();
    if (result != 0
#if defined(__OpenBSD__) || defined(__FreeBSD__) || defined(__NetBSD__) || defined(__DragonFly__)
  	  && result != 2	// can only set local date, which is okay
#endif
      ) {
      ret |= ERROR_DTIME_DATE;
      return ret;
    }
    return ret;
}

// on non-Solaris systems which do not use /etc/timezone?
static int Tzone_save_set( const QString& selectedzone )
{
    int ret = 0;
#if defined(USE_SOLARIS)	// MARCO

        KTemporaryFile tf;
        tf.setPrefix("kde-tzone");
        tf.open();
        QTextStream ts(&tf);

        QFile fTimezoneFile(INITFILE);
        bool updatedFile = false;

        if (fTimezoneFile.open(QIODevice::ReadOnly))
        {
            bool found = false;

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
            fTimezoneFile.close();
        }

        if (updatedFile)
        {
            ts.device()->reset();
            fTimezoneFile.remove();

            if (fTimezoneFile.open(QIODevice::WriteOnly | QIODevice::Truncate))
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
        QString tz = "/usr/share/zoneinfo/" + selectedzone;

        kDebug() << "Set time zone " << tz;

        if( !KStandardDirs::findExe( "zic" ).isEmpty())
        {
            KProcess::execute("zic", QStringList() << "-l" << selectedzone);
        }
        else
        {
            QFile fTimezoneFile("/etc/timezone");

            if (fTimezoneFile.open(QIODevice::WriteOnly | QIODevice::Truncate) )
            {
                QTextStream t(&fTimezoneFile);
                t << selectedzone;
                fTimezoneFile.close();
            }

            if (!QFile::remove("/etc/localtime"))
            {
                ret |= ERROR_TZONE;
            }
            else
                if (!QFile::copy(tz,"/etc/localtime"))
                    ret |= ERROR_TZONE;
        }

        QString val = ':' + tz;
#endif // !USE_SOLARIS

        setenv("TZ", val.toAscii(), 1);
        tzset();

    return ret;
}

static int Tzone_save_reset()
{
#if !defined(USE_SOLARIS) // Do not update the System!
        unlink( "/etc/timezone" );
        unlink( "/etc/localtime" );

        setenv("TZ", "", 1);
        tzset();
#endif // !USE SOLARIS
    return 0;
}


int main( int argc, char* argv[] )
{
  if( getuid() != 0 ) {
    fprintf( stderr, "Needs to be called as root.\n" );
    return ERROR_CALL;
  }
  bool ntp = false;
  QStringList ntpServerList;
  bool ntpEnabled;
  bool date = false;
  QString datestr;
  bool tz = false;
  QString timezone;
  bool tzreset = false;
  QStringList args;
  for( int pos = 1;
       pos < argc;
       ++pos )
    args.append( argv[ pos ] ); // convert to QStringList first to protect against possible overflows
  while( !args.isEmpty()) {
    QString arg = args.takeFirst();
    if( arg == "ntp" && !args.isEmpty()) {
      int ntpCount = args.takeFirst().toInt();
      if( ntpCount >= 0 && ntpCount <= args.count()) {
        for( int i = 0;
             i < ntpCount;
             ++i ) {
          ntpServerList.append( args.takeFirst());
        }
      }
      if( args.isEmpty()) {
        fprintf( stderr, "Wrong arguments!\n" );
        exit( ERROR_CALL );
      }
      ntpEnabled = args.takeFirst() == "enabled";
      ntp = true;
    } else if( arg == "date" && !args.isEmpty()) {
      datestr = args.takeFirst();
      date = true;
    } else if( arg == "tz" && !args.isEmpty()) {
      timezone = args.takeFirst();
      tz = true;
    } else if( arg == "tzreset" ) {
      tzreset = true;
    } else {
      fprintf( stderr, "Wrong arguments!\n" );
      exit( ERROR_CALL );
    }
  }
  int ret = 0; // error code
//  The order here is important
  if( ntp )
    ret |= Dtime_save_ntp( ntpServerList, ntpEnabled );
  if( date )
    ret |= Dtime_save_date( datestr );
  if( tz )
    ret |= Tzone_save_set( timezone );
  if( tzreset )
    ret |= Tzone_save_reset();
  return ret;
}
