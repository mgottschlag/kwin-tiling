/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <kio/job.h>
#include <kio/netaccess.h>
#include <kstandarddirs.h>
#include <kdesktopfile.h>
#include <kglobalsettings.h>
#include <kapplication.h>
#include <kprocess.h>
#include <kmessagebox.h>
#include <klocale.h>
#include <kdebug.h>
#include "kdesktopsettings.h"

#include <QDir>
#include <QFile>
#include <QRegExp>

#include <sys/stat.h>
#include <sys/types.h>
#include <fcntl.h>
#include <unistd.h>
#include <dirent.h>
#include <stdlib.h>
#include <errno.h>
#include <ksimpleconfig.h>
#include <kglobal.h>

// for multihead
extern int kdesktop_screen_number;


/**
 * Test if a directory exists, create otherwise
 * @param _name full path of the directory
 * @param _showMsg show a message box if we created the dir
 * @return true if the dir was just created (e.g. so that we can populate it)
 */
static bool testDir( const QString &_name )
{
  DIR *dp;
  dp = opendir( QFile::encodeName(_name) );
  if ( dp == NULL )
  {
    QString m = _name;
    if ( m.endsWith( "/" ) )
      m.truncate( m.length() - 1 );
    QByteArray path = QFile::encodeName(m);

    bool ok = ::mkdir( path, 0755 ) == 0;
    if ( !ok && errno == EEXIST ) {
        int ret = KMessageBox::warningYesNo( 0, i18n("%1 is a file, but KDE needs it to be a directory; move it to %2.orig and create directory?", m, m), QString(), KGuiItem(i18n("Move It")), KGuiItem(i18n("Do Not Move")) );
        if ( ret == KMessageBox::Yes ) {
            if ( ::rename( path, path + ".orig" ) == 0 ) {
                ok = ::mkdir( path, 0755 ) == 0;
            } else {
                // foo.orig existed already. How likely is that?
                ok = false;
            }
        } else {
            return false;
        }
    }
    if ( !ok )
    {
        KMessageBox::sorry( 0, i18n( "Could not create directory %1; check for permissions or reconfigure the desktop to use another path." ,  m ) );
        return false;
    }
    return true;
  }
  else // exists already
  {
    closedir( dp );
    return false;
  }
}

/**
 * Copy a standard .directory file to a user's directory
 * @param fileName destination file name
 * @param dir destination directory
 * @param force if false, don't copy if destination file already exists
 */
static void copyDirectoryFile(const QString &fileName, const QString& dir, bool force)
{
  if (force || !QFile::exists(dir + "/.directory")) {
    QString cmd = "cp ";
    cmd += KProcess::quote(KStandardDirs::locate("data", QString("kdesktop/") + fileName));
    cmd += ' ';
    cmd += KProcess::quote(dir+"/.directory");
    system( QFile::encodeName(cmd) );
  }
}

static void copyFile( const QString& src, const QString& dest )
{
    QByteArray cmd = "cp ";
    cmd += QFile::encodeName(KProcess::quote(src));
    cmd += ' ';
    cmd += QFile::encodeName(KProcess::quote(dest));
    system( cmd );
}

static QString realDesktopPath()
{
    QString desktopPath = KGlobalSettings::desktopPath();
    if (kdesktop_screen_number != 0) {
	QString dn = "Desktop";
	dn += QString::number(kdesktop_screen_number);
	desktopPath.replace("Desktop", dn);
    }
    return desktopPath;
}

/**
 * Copy all links from DesktopLinks/ to the desktop, appropriately renamed
 * (to the contents of the translated Name field)
 */
static void copyDesktopLinks()
{
    KSharedConfig::Ptr config = KGlobal::config();
    config->setGroup("General");
    if (!config->readEntry("CopyDesktopLinks", true))
       return;

    QStringList list =
	KGlobal::dirs()->findAllResources("appdata", "DesktopLinks/*", KStandardDirs::NoDuplicates);

    QString desktopPath = realDesktopPath();

    for (QStringList::ConstIterator it = list.begin(); it != list.end(); it++) {
        KDesktopFile desk( *it );
        if (desk.readEntry("Hidden", false))
           continue;
        copyFile( *it, desktopPath );
    }
}

/**
 * @return true if this is the first time
 * kdesktop is run for the current release
 * WARNING : call only once !!! (second call will always return false !)
 */
static bool isNewRelease()
{
    bool bNewRelease = false;

    int versionMajor = KDesktopSettings::kDEVersionMajor();
    int versionMinor = KDesktopSettings::kDEVersionMinor();
    int versionRelease = KDesktopSettings::kDEVersionRelease();

    if( versionMajor < KDE_VERSION_MAJOR )
        bNewRelease = true;
    else if( versionMinor < KDE_VERSION_MINOR )
        bNewRelease = true;
    else if( versionRelease < KDE_VERSION_RELEASE )
        bNewRelease = true;

    if( bNewRelease ) {
      KDesktopSettings::setKDEVersionMajor( KDE_VERSION_MAJOR );
      KDesktopSettings::setKDEVersionMinor( KDE_VERSION_MINOR );
      KDesktopSettings::setKDEVersionRelease( KDE_VERSION_RELEASE );
      KDesktopSettings::writeConfig();
    }

    return bNewRelease;
}

/**
 * Create, if necessary, some directories in user's .kde/,
 * copy default .directory files there, as well as templates files.
 * Called by kdesktop on startup.
 */
void testLocalInstallation()
{
    const bool newRelease = isNewRelease();

    const QString desktopPath = realDesktopPath();
    const bool emptyDesktop = testDir( desktopPath );

    // Do not force copying that one (it would lose the icon positions)
    copyDirectoryFile("directory.desktop", desktopPath, false);

    testDir( KGlobalSettings::autostartPath() );
    // we force the copying in case of a new release, to install new translations....
    copyDirectoryFile("directory.autostart", KGlobalSettings::autostartPath(), newRelease);

    if (emptyDesktop)
	copyDesktopLinks();

    // Take care of creating or updating trash.desktop
    const QString trashDir = KGlobal::dirs()->localxdgdatadir() + "Trash";
    const bool firstTimeWithNewTrash = !QFile::exists( trashDir );
    const QString trashDesktopPath = desktopPath + "/trash.desktop";
    const bool trashDesktopExists = QFile::exists( trashDesktopPath );
    const bool installNewTrashi18n = newRelease && trashDesktopExists; // not if deleted by user
    if ( emptyDesktop || firstTimeWithNewTrash || installNewTrashi18n ) {
        QString oldIcon, oldEmptyIcon;
        if ( trashDesktopExists ) {
            KDesktopFile trashDesktop( trashDesktopPath, true );
            oldIcon = trashDesktop.readIcon();
            oldEmptyIcon = trashDesktop.readEntry( "EmptyIcon" );
        }
        copyFile( KStandardDirs::locate( "data", "kdesktop/directory.trash" ), trashDesktopPath );
        if ( trashDesktopExists ) {
            KDesktopFile trashDesktop( trashDesktopPath );
            trashDesktop.writeEntry( "Icon", oldIcon );
            trashDesktop.writeEntry( "EmptyIcon", oldEmptyIcon );
            trashDesktop.sync();
        }
    }

    if ( firstTimeWithNewTrash ) { // migrate pre-kde-3.4 trash contents
        QByteArray packedArgs;
        QDataStream stream( &packedArgs, QIODevice::WriteOnly );

        stream.setVersion(QDataStream::Qt_3_1);
        stream << (int)2;
        KIO::Job* job = KIO::special( KUrl("trash:/"), packedArgs );
        (void)KIO::NetAccess::synchronousRun( job, 0 );

        // OK the only thing missing is to convert the icon position...
        KSimpleConfig cfg( KStandardDirs::locateLocal("appdata", "IconPositions") );
        if ( cfg.hasGroup( "IconPosition::Trash" ) && !cfg.hasGroup( "IconPosition::trash.desktop" ) ) {
            const QMap<QString, QString> entries = cfg.entryMap( "IconPosition::Trash" );
            cfg.setGroup( "IconPosition::trash.desktop" );
            for( QMap<QString,QString>::ConstIterator it = entries.begin(); it != entries.end(); ++it ) {
                cfg.writeEntry( it.key(), it.value() );
            }
        }
    }
}

