#ifndef __MISC_H__
#define __MISC_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CMisc
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
////////////////////////////////////////////////////////////////////////////////

#include <unistd.h>
#include <sys/types.h>
#include <qstring.h>
#include <qstringlist.h>
#include <qtl.h>
#include <kglobal.h>
#include <kinstance.h>
#include <kstddirs.h>

class QComboBox;

class CMisc
{
    public:

    enum EConstants
    {
        MAX_SUB_DIRS = 4
    };

    static bool         dExists(const QString &d);
    static bool         fExists(const QString &f);
    static bool         fWritable(const QString &f);
    static bool         dWritable(const QString &d);
    static bool         dHasSubDirs(const QString &d);
    static bool         dContainsTTorT1Fonts(const QString &d);
    static QString      getDir(const QString &f);
    static unsigned int getNumItems(const QString &d);
    static unsigned int countFonts(const QString &d);
    static bool         createDir(const QString &dir)                        { return doCmd("mkdir", dir); }
    static bool         removeDir(const QString &dir)                        { return doCmd("rmdir", dir); }
    static bool         removeFile(const QString &file)                      { return doCmd("rm", "-f", file); }
    static bool         moveFile(const QString &file, const QString &dest)   { return doCmd("mv", "-f", file, dest); }
    static bool         copyFile(const QString &src, const QString &file, const QString &dest)
                            { return doCmd("cp", "-f", src+file, dest) && doCmd("chmod", "+w", dest+file); }
    static bool         linkFile(const QString &source, const QString &dest) { return doCmd("ln", "-s", source, dest); }
    static bool         doCmd(const QString &cmd, const QString &p1=QString::null, const QString &p2=QString::null, const QString &p3=QString::null);
    static bool         doCmdStr(const QString &cmd);
    static QString      changeExt(const QString &f, const QString &newExt);
    static QString      afmName(const QString &f) { return changeExt(f, "afm"); }
    static QString      locate(QString file) { return KGlobal::instance()->dirs()->findResource("data", "kcmfontinst/"+file); }
    static QStringList  locateAll(QString dir, QString type) { return KGlobal::instance()->dirs()->findAllResources("data", "kcmfontinst/"+dir+"/"+"*."+type); }
    static QString      removeSymbols(const QString &str);
    static QString      shortName(const QString &dir);
    static int          findIndex(const QComboBox *box, const QString &str);
    static bool         root() { return getuid()==0 ? true : false; }
    static int          stricmp(const char *s1, const char *s2);
    static bool         find(const QStringList &list, const QString &str) { return list.end()!=qFind(list.begin(), list.end(), str); }
};

#endif
