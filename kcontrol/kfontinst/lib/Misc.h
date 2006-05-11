#ifndef __MISC_H__
#define __MISC_H__

////////////////////////////////////////////////////////////////////////////////
//
// Namespace     : KFI::Misc
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 01/05/2001
// Version       : $Revision$ $Date$
//
////////////////////////////////////////////////////////////////////////////////
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <QString>
#include <qstringlist.h>
#include <kurl.h>

class QWidget;

namespace KFI
{

namespace Misc
{
    enum EConstants
    {
        FILE_PERMS   = 0644,
        DIR_PERMS    = 0755
    };

    extern KDE_EXPORT bool    check(const QString &path, unsigned int fmt, bool checkW=false);
    inline KDE_EXPORT bool    fExists(const QString &p)     { return check(p, S_IFREG, false); }
    inline KDE_EXPORT bool    dExists(const QString &p)     { return check(p, S_IFDIR, false); }
    inline KDE_EXPORT bool    fWritable(const QString &p)   { return check(p, S_IFREG, true); }
    inline KDE_EXPORT bool    dWritable(const QString &p)   { return check(p, S_IFDIR, true); }
    inline KDE_EXPORT bool    isLink(const QString &i)      { return check(i, S_IFLNK, false); }
    extern KDE_EXPORT QString linkedTo(const QString &i);
    extern KDE_EXPORT QString dirSyntax(const QString &d);  // Has trailing slash:  /file/path/
    extern KDE_EXPORT QString xDirSyntax(const QString &d); // No trailing slash:   /file/path
    inline KDE_EXPORT QString fileSyntax(const QString &f)  { return xDirSyntax(f); }
    extern KDE_EXPORT QString getDir(const QString &f);
    extern KDE_EXPORT QString getFile(const QString &f);
    extern KDE_EXPORT bool    createDir(const QString &dir);
    extern KDE_EXPORT QString changeExt(const QString &f, const QString &newExt);
    extern KDE_EXPORT bool    doCmd(const QString &cmd, const QString &p1=QString(), const QString &p2=QString(), const QString &p3=QString());
    inline KDE_EXPORT bool    root() { return 0==getuid(); }
    extern KDE_EXPORT void    getAssociatedUrls(const KUrl &url, KUrl::List &list, bool afmAndPfm=true, QWidget *widget=NULL);
    extern KDE_EXPORT void    createBackup(const QString &f);
    extern KDE_EXPORT time_t  getTimeStamp(const QString &item);
}

}

#endif
