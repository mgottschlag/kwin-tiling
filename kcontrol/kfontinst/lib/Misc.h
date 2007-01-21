#ifndef __MISC_H__
#define __MISC_H__

/*
 * KFontInst - KDE Font Installer
 *
 * (c) 2003-2006 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License version 2 as published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>
#include <QString>
#include <QStringList>
#include <QDataStream>
#include <kurl.h>
#include "KfiConstants.h"

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

    struct TFont
    {
        TFont(const QString &f=QString(), unsigned int s=KFI_NO_STYLE_INFO) : family(f), styleInfo(s) { }

        bool operator==(const TFont &o) const { return o.styleInfo==styleInfo && o.family==family; }

        QString      family;
        unsigned int styleInfo;
    };

    extern KDE_EXPORT QString prettyUrl(const KUrl &url);
    inline KDE_EXPORT bool    isHidden(const QString &f)    { return QChar('.')==f[0]; }
    inline KDE_EXPORT bool    isHidden(const KUrl &url)     { return isHidden(url.fileName()); }
    extern KDE_EXPORT bool    check(const QString &path, unsigned int fmt, bool checkW=false);
    inline KDE_EXPORT bool    fExists(const QString &p)     { return check(p, S_IFREG, false); }
    inline KDE_EXPORT bool    dExists(const QString &p)     { return check(p, S_IFDIR, false); }
    inline KDE_EXPORT bool    fWritable(const QString &p)   { return check(p, S_IFREG, true); }
    inline KDE_EXPORT bool    dWritable(const QString &p)   { return check(p, S_IFDIR, true); }
    extern KDE_EXPORT QString linkedTo(const QString &i);
    extern KDE_EXPORT QString dirSyntax(const QString &d);  // Has trailing slash:  /file/path/
    extern KDE_EXPORT QString fileSyntax(const QString &f);
    extern KDE_EXPORT QString getDir(const QString &f);
    extern KDE_EXPORT QString getFile(const QString &f);
    extern KDE_EXPORT bool    createDir(const QString &dir);
    extern KDE_EXPORT QString changeExt(const QString &f, const QString &newExt);
    extern KDE_EXPORT bool    doCmd(const QString &cmd, const QString &p1=QString(),
                                    const QString &p2=QString(), const QString &p3=QString());
    inline KDE_EXPORT bool    root() { return 0==getuid(); }
    extern KDE_EXPORT void    getAssociatedFiles(const QString &file, QStringList &list,
                                                 bool afmAndPfm=true);
    extern KDE_EXPORT time_t  getTimeStamp(const QString &item);
    extern KDE_EXPORT QString getFolder(const QString &defaultDir, const QString &root,
                                        QStringList &dirs);
    extern KDE_EXPORT bool    checkExt(const QString &fname, const QString &ext);
    extern KDE_EXPORT bool    isMetrics(const QString &str);
    inline KDE_EXPORT bool    isMetrics(const KUrl &url) { return isMetrics(url.fileName()); }
    inline KDE_EXPORT bool    isPackage(const QString &file)
                      { return file.indexOf(KFI_FONTS_PACKAGE)==(file.length()-KFI_FONTS_PACKAGE_LEN); }
    inline KDE_EXPORT bool    isGroup(const QString &file)
                      { return file.indexOf(KFI_FONTS_GROUP)==(file.length()-KFI_FONTS_GROUP_LEN); }
    extern KDE_EXPORT int     getIntQueryVal(const KUrl &url, const char *key, int defVal);
    extern KDE_EXPORT bool    printable(const QString &mime);
    inline KDE_EXPORT QString unhide(const QString &f) { return '.'==f[0] ? f.mid(1) : f; }
    extern KDE_EXPORT uint    qHash(const TFont &key);
    extern KDE_EXPORT bool    configureForX11(const QString &dir);
}

}

inline KDE_EXPORT QDataStream & operator<<(QDataStream &ds, const KFI::Misc::TFont &font)
{
    ds << font.family << font.styleInfo;
    return ds;
}

inline KDE_EXPORT QDataStream & operator>>(QDataStream &ds, KFI::Misc::TFont &font)
{
    ds >> font.family >> font.styleInfo;
    return ds;
}

#endif
