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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif
#include "Misc.h"
#include <qptrlist.h>
#include <qstring.h>
#include <qstringlist.h>
#include <time.h>

class CXConfig
{
    private:

    struct TPath
    {
        enum EType
        {
            DIR,
            FONT_SERVER
            // NOTE: In future XF86Config may allow "fontconfig" to be specified as a path!
        };

        TPath(const QString &d, bool u, EType t, bool o)
           : dir(CMisc::dirSyntax(d)), unscaled(u), toBeRemoved(false), orig(o), type(t) {}

        static EType getType(const QString &d);

        QString dir;
        bool    unscaled,
                toBeRemoved,   // Whether dir should be removed when saving file
                orig;          // Was dir in file when read?
        EType   type;
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
        unsigned int        xlfdCount() { return itsXlfdCount; }

        private:

        TEntry * findEntry(const QString &fname);
        TEntry * getEntry(TEntry **current, const QString &filename);

        private:

        QPtrList<TEntry> itsEntries;
        unsigned int     itsXlfdCount;
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

#ifdef HAVE_FONTCONFIG
    static bool configureDir(const QString &dir);
#else
    static bool configureDir(const QString &dir, QStringList &symbolFamilies);
#endif

    bool ok()                           { return itsOk; }
    bool writable()                     { return itsWritable; }
    bool readConfig();
    bool writeConfig();
    bool madeChanges();
    bool inPath(const QString &dir);
    bool subInPath(const QString &dir);
    void addPath(const QString &dir, bool unscaled=false);
    void removePath(const QString &dir);
    bool getDirs(QStringList &list);
    bool xfsInPath();
    void refreshPaths();
    void restart();

    private:

    bool readFontpaths();
    bool writeFontpaths();
    bool processXf86(bool read);
    bool processXfs(bool read);

    TPath * findPath(const QString &dir);

#ifdef HAVE_FONTCONFIG
    static bool createFontsDotDir(const QString &dir);
#else
    static bool createFontsDotDir(const QString &dir, QStringList &symbolFamilies);
#endif

    private:

    EType           itsType;
    QPtrList<TPath> itsPaths;
    QString         itsFileName,
                    itsInsertPos;
    bool            itsOk,
                    itsWritable;
    time_t          itsTime;
};

#endif
