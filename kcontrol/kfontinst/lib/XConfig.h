#ifndef __X_CONFIG_H__
#define __X_CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXConfig
// Author        : Craig Drummond
// Project       : K Font Installer
// Creation Date : 05/05/2001
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
///////////////////////////////////////////////////////////////////////////////

#include "Misc.h"
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>

class CXConfig
{
    private:

    struct TPath
    {
        TPath(const QString &d, bool u, bool dis, bool o=false)
           : dir(CMisc::dirSyntax(d)), unscaled(u), origUnscaled(u), disabled(dis), orig(o) {}

        QString dir;
        bool    unscaled,
                origUnscaled,
                disabled,   // Whether dir should be disabled when saving file
                orig;       // Was dir in file when read?
    };

    class CFontsFile
    {
        private:

        struct TEntry
        {
            TEntry(const QString &fname) : filename(fname) {}

            QString     filename;
            QStringList xlfds;
        };

        public:

        CFontsFile(const char *fname);

        const QStringList * getXlfds(const QString &fname);

        private:

        TEntry * findEntry(const QString &fname);
        TEntry * getEntry(TEntry **current, const QString &filename);

        private:

        QPtrList<TEntry> itsEntries;
    };

    public:

    enum EType
    {
        XFS,
        XF86,
        KFI
    };

    public:

    CXConfig(EType type, const QString &file);

    static bool configureDir(const QString &dir, QStringList &symbolFamilies);

    bool ok()                           { return itsOk; }
    bool writable()                     { return itsWritable; }
    bool readConfig();
    bool writeConfig();
    bool madeChanges();
    bool inPath(const QString &dir);
    bool subInPath(const QString &dir);
#if 0
    bool isUnscaled(const QString &dir);
    void setUnscaled(const QString &dir, bool unscaled);
#endif
    void addPath(const QString &dir, bool unscaled=false);
    void removePath(const QString &dir);
    bool getDirs(QStringList &list);
    void refreshPaths();
    void restart();

    private:

    bool readFontpaths();
    bool writeFontpaths();
    bool processXf86(bool read);
    bool processXfs(bool read);

    TPath * findPath(const QString &dir);

    static bool createFontsDotDir(const QString &dir, QStringList &symbolFamilies);

    private:

    EType           itsType;
    QPtrList<TPath> itsPaths;
    QString         itsFileName,
                    itsInsertPos;
    bool            itsOk,
                    itsWritable;
};

#endif
