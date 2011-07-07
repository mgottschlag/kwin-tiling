#ifndef __KIO_FONTS_H__
#define __KIO_FONTS_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2007 Craig Drummond <craig@kde.org>
 *
 * ----
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
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

#include <KDE/KIO/SlaveBase>
#include <QtDBus/QtDBus>
#include <sys/types.h>

class KTempDir;

namespace KFI
{

class Family;
class Style;
class FontInstInterface;

class CKioFonts : public KIO::SlaveBase
{
    public:

    enum EFolder
    {
        FOLDER_USER,
        FOLDER_SYS,
        FOLDER_ROOT,
        FOLDER_UNKNOWN
    };

    CKioFonts(const QByteArray &pool, const QByteArray &app);
    virtual ~CKioFonts();

    void listDir(const KUrl &url);
    void put(const KUrl &url, int permissions, KIO::JobFlags flags);
    void get(const KUrl &url);
    void del(const KUrl &url, bool isFile);
    void copy(const KUrl &src, const KUrl &dest, int mode, KIO::JobFlags flags);
    void rename(const KUrl &src, const KUrl &dest, KIO::JobFlags flags);
    void stat(const KUrl &url);
    void special(const QByteArray &a);

    private:

    int     listFolder(KIO::UDSEntry &entry, EFolder folder);
    QString getUserName(uid_t uid);
    QString getGroupName(gid_t gid);
    bool    createStatEntry(KIO::UDSEntry &entry, const KUrl &url, EFolder folder);
    void    createUDSEntry(KIO::UDSEntry &entry, EFolder folder);
    bool    createUDSEntry(KIO::UDSEntry &entry, EFolder folder, const Family &family, const Style &style);
    Family  getFont(const KUrl &url, EFolder folder);
    void    handleResp(int resp, const QString &file, const QString &tempFile=QString(), bool destIsSystem=false);

    private:

    FontInstInterface       *itsInterface;
    KTempDir                *itsTempDir;
    QHash<uid_t, QString>   itsUserCache;
    QHash<gid_t, QString>   itsGroupCache;
};

}

#endif
