#ifndef __MISC_H__
#define __MISC_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CMisc
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
#include <kstandarddirs.h>

class CMisc
{
    public:

    enum EConstants
    {
        MAX_SUB_DIRS = 4,
        FILE_PERMS   = 0644,
        DIR_PERMS    = 0755
    };

    static bool    fExists(const QString &p, bool format=false);
    static bool    dExists(const QString &p)     { return check(p, S_IFDIR, false); }
    static bool    fWritable(const QString &p)   { return check(p, S_IFREG, true); }
    static bool    dWritable(const QString &p)   { return check(p, S_IFDIR, true); }
    static bool    isLink(const QString &i)      { return check(i, S_IFLNK, false); }
    static QString linkedTo(const QString &i);
    static QString dirSyntax(const QString &d);
    static QString xDirSyntax(const QString &d);
    static QString getDir(const QString &f);
    static QString getFile(const QString &f);
    static bool    createDir(const QString &dir) { return KStandardDirs::makeDir(dir); }
    static bool    doCmd(const QString &cmd, const QString &p1=QString::null, const QString &p2=QString::null, const QString &p3=QString::null);
    static QString changeExt(const QString &f, const QString &newExt);
    static QString afmName(const QString &f)     { return changeExt(f, "afm"); }
    static QString removeSymbols(const QString &str);
    static bool    root() { return getuid()==0 ? true : false; }
    static void    createBackup(const QString &f);
    static int     stricmp(const char *s1, const char *s2);
    static QString getName(const QString &f);
    static QString getSect(const QString &f)     { return f.section('/', 1, 1); }
    static QString getSub(const QString &f)      { return root() ? f : f.section('/', 2); }
    static void    removeAssociatedFiles(const QString realPath, bool d=false);
    static bool    hidden(const QString &u, bool dir=false);
    static time_t  getTimeStamp(const QString &item);
    static void    setTimeStamps(const QString &ds);
    static QString formatFileName(const QString &p);

    private:

    static bool    check(const QString &path, unsigned int fmt, bool checkW=false);
};

#endif
