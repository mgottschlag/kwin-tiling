#ifndef __X_CONFIG_H__
#define __X_CONFIG_H__

////////////////////////////////////////////////////////////////////////////////
//
// Class Name    : CXConfig
// Author        : Craig Drummond
// Project       : K Font Installer (kfontinst-kcontrol)
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
// Foundation, Inc., 675 Mass Ave, Cambridge, MA 02139, USA.
//
////////////////////////////////////////////////////////////////////////////////
// (C) Craig Drummond, 2001
///////////////////////////////////////////////////////////////////////////////

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include "Misc.h"

#include <qobject.h>
#include <qglobal.h>
#if QT_VERSION >= 300
#include <qptrlist.h>
#else
#include <qlist.h>
#endif
#include <qstring.h>
#include <qstringlist.h>

class CXConfig : public QObject
{
    Q_OBJECT

    private:

    enum EType
    {
        NONE,
        KFONTINST,
        XF86CONFIG,
        XFS
    };

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

    public:

    CXConfig();

    bool go(const QString &dir, QStringList &symbolFamilies);

    bool ok()                           { return NONE!=itsType; }
    bool custom()                       { return KFONTINST==itsType; }
    bool writable()                     { return itsWritable; }

    bool readConfig();
    bool writeConfig();
    bool madeChanges();
    bool inPath(const QString &dir);
    bool isUnscaled(const QString &dir);
    void setUnscaled(const QString &dir, bool unscaled);
    void addPath(const QString &dir, bool unscaled=false);
    void removePath(const QString &dir);
    bool getTTandT1Dirs(QStringList &list);
    void refreshPaths();

    signals:

    void step(const QString &);

    private:

    bool readFontpaths();
    bool writeFontpaths();
    bool readXF86Config();
    bool writeXF86Config();
    bool processXfs(const QString &fname, bool read);
    bool readXfsConfig();
    bool writeXfsConfig();

    TPath * findPath(const QString &dir);

    bool createFontsDotDir(const QString &dir, QStringList &symbolFamilies);

    private:

#if QT_VERSION >= 300
    QPtrList<TPath> itsPaths;
#else
    QList<TPath> itsPaths;
#endif
    EType        itsType;
    QString      itsInsertPos;
    bool         itsWritable;
};

#endif
