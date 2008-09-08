#ifndef __FONTINST_H__
#define __FONTINST_H__

/*
 * KFontInst - KDE Font Installer
 *
 * Copyright 2003-2008 Craig Drummond <craig@kde.org>
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

#include <QtCore/QObject>
#include <KDE/KComponentData>
#include "DisabledFonts.h"

class QTimer;

using namespace KFI;

class FontInst : public QObject
{
    Q_OBJECT
    Q_CLASSINFO("D-Bus Interface", "org.kde.fontinst")

    public:

    enum EStatus
    {
        CommandOk,
        CommandFailed,
        NotAuthorised
    };

    FontInst(QObject *parent);
    virtual ~FontInst();
 
    public Q_SLOTS:

    int  disableFont(uint key, const QString &family, uint style, qulonglong writingSystems,
                     uint face, const QStringList &fileData);
    int  enableFont(uint key, const QString &family, uint style);
    int  deleteDisabledFont(uint key, const QString &family, uint style);
    int  reloadDisabledList(uint key);
    int  addToFc(uint key, const QString &dir);
    int  configureX(uint key, const QString &dir);
    int  copyFont(uint key, const QString &src, const QString &dest);
    int  moveFont(uint key, const QString &src, const QString &dest, uint uid, uint gid);
    int  deleteFont(uint key, const QString &font);
    int  createDir(uint key, const QString &dir);
    int  createAfm(uint key, const QString &font);
    int  fcCache(uint key);

    private Q_SLOTS:

    void timeout();

    private:

    void refreshDirList();
    bool pathIsOk(const QString &path);

    private:

    friend class CCommandThread;

    QTimer         *itsTimer;
    CDisabledFonts itsDisabledFonts;
    QStringList    itsFcDirs;
};

#endif
