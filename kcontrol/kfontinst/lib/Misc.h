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
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001, 2002, 2003
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <qstring.h>
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

    extern bool    check(const QString &path, unsigned int fmt, bool checkW=false);
    inline bool    fExists(const QString &p)     { return check(p, S_IFREG, false); }
    inline bool    dExists(const QString &p)     { return check(p, S_IFDIR, false); }
    inline bool    fWritable(const QString &p)   { return check(p, S_IFREG, true); }
    inline bool    dWritable(const QString &p)   { return check(p, S_IFDIR, true); }
    inline bool    isLink(const QString &i)      { return check(i, S_IFLNK, false); }
    extern QString linkedTo(const QString &i);
    extern QString dirSyntax(const QString &d);  // Has trailing slash:  /file/path/
    extern QString xDirSyntax(const QString &d); // No trailing slash:   /file/path
    inline QString fileSyntax(const QString &f)  { return xDirSyntax(f); }
    extern QString getDir(const QString &f);
    extern QString getFile(const QString &f);
    extern bool    createDir(const QString &dir);
    extern QString changeExt(const QString &f, const QString &newExt);
    extern bool    doCmd(const QString &cmd, const QString &p1=QString::null, const QString &p2=QString::null, const QString &p3=QString::null);
    inline bool    root() { return 0==getuid(); }
    extern void    getAssociatedUrls(const KURL &url, KURL::List &list, bool afmAndPfm=true, QWidget *widget=NULL);
    extern void    createBackup(const QString &f);
    extern time_t  getTimeStamp(const QString &item);
};

};

#endif
