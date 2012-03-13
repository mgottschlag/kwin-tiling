/*
 *   Copyright (C) 2010 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#ifndef SYSTEM_INFO_H_
#define SYSTEM_INFO_H_

#include <stdlib.h>
#include <config-workspace.h>

#include <QDir>
#include <QString>

QString homeDir()
{
    const char * kdehome = getenv("KDEHOME");
    const char * home = getenv("HOME");
    if (!kdehome || !kdehome[0]) {
        return QString() + home + "/" + KDE_DEFAULT_HOME;
    }

    return kdehome;
}

QString systemDir()
{
    return KDE_DATADIR;
}

QString themeDir(QString theme)
{
    QString path;

    path = homeDir() + "/share/apps/ksplash/Themes/" + theme;

    if (!QDir(path).exists()) {
        path = systemDir() + "/ksplash/Themes/" + theme;
    }

    return path;
}

#endif // SYSTEM_INFO_H_


