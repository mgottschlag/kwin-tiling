/*
 *  helper.cpp
 *
 *  Copyright (C) 2010 Igor Krivenko <igor@shg.ru>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 *
 */

/*

 A helper that's run using KAuth and does the system modifications.

*/

#include "helper.h"

#include <config-workspace.h>

#include <QFile>
#include <QDir>
#include <KConfig>
#include <KConfigGroup>
#include <KStandardDirs>

bool secureCopy(const QString &from, const QString &to)
{
    QFile srcFile(from);
    if (!srcFile.open(QIODevice::ReadOnly))
        return false;

    // Security check: we don't want to expose contents of files like /etc/shadow
    if (!(srcFile.permissions() & QFile::ReadOther))
        return false;

    QFile dstFile(to);
    if (!dstFile.open(QIODevice::WriteOnly))
        return false;

    const quint64 maxBlockSize = 102400;
    while (!srcFile.atEnd())
        if (dstFile.write(srcFile.read(maxBlockSize)) == -1)
            return false;

    if (!dstFile.setPermissions(
                QFile::WriteUser | QFile::ReadUser | QFile::ReadGroup | QFile::ReadOther))
        return false;

    return true;
}

bool createDir(const QString &path)
{
    QDir testDir(path);
    return (testDir.exists()) || (testDir.mkpath(path));
}

bool removeDirTree(const QString &rootDir)
{
    QDir dir(rootDir);
    foreach (const QFileInfo &entry, dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot))
        if (entry.isDir())
            removeDirTree(entry.absoluteFilePath());
        else
            dir.remove(entry.fileName());

    return dir.rmdir(rootDir);
}

bool installDirTree(const QString &srcRootDir, const QString &dstRootDir)
{
    QDir srcDir(srcRootDir);
    QDir dstDir(dstRootDir);

    if (!dstDir.mkdir(dstRootDir))
        return false;

    if (!QFile::setPermissions(dstRootDir,
               QFile::WriteUser | QFile::ReadUser | QFile::ExeUser |
               QFile::ReadGroup | QFile::ExeGroup |
               QFile::ReadOther | QFile::ExeOther))
        return false;

    bool res = true;
    foreach (const QFileInfo &entry, srcDir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot)) {
        QString srcFilePath = entry.absoluteFilePath();
        QString dstFilePath = dstDir.absolutePath() + '/' + entry.fileName();

        if (entry.isDir()) {
            if (!installDirTree(srcFilePath, dstFilePath))
                res = false;
        } else {
            if (!secureCopy(srcFilePath, dstFilePath))
                res = false;
        }
    }
    return res;
}

ActionReply Helper::createReply(int code, const QVariantMap *returnData)
{
    ActionReply reply;

    if (code) {
        reply = ActionReply::HelperError;
        reply.setErrorCode(code);
    } else {
        reply = ActionReply::SuccessReply;
    }

    if (returnData)
        reply.setData(*returnData);

    return reply;
}

ActionReply Helper::save(const QVariantMap &args)
{
    QString tempConfigName = args.value("tempkdmrcfile").toString();
    QString tempBackgroundConfigName = args.value("tempbackgroundrcfile").toString();

    QString systemConfigName = QString::fromLatin1(KDE_CONFDIR "/kdm/kdmrc");
    QString systemBackgroundConfigName = KConfig(systemConfigName, KConfig::SimpleConfig)
            .group("X-*-Greeter").readEntry("BackgroundCfg", KDE_CONFDIR "/kdm/backgroundrc");

    int code = 0;

    if (!secureCopy(tempConfigName, systemConfigName))
        code |= KdmrcInstallError;
    if (!secureCopy(tempBackgroundConfigName, systemBackgroundConfigName))
        code |= BackgroundrcInstallError;

    return createReply(code);
}

bool Helper::removeFace(const QString &facesDir, const QString &user)
{
    // Security check
    if (user.contains('/'))
        return false;

    return QFile::remove(facesDir + user + ".face.icon");
}

bool Helper::installFace(const QString &facesDir, const QString &user, const QString &sourceFile)
{
    // Security check
    if (user.contains('/'))
        return false;

    return secureCopy(sourceFile, facesDir + user + ".face.icon");
}

ActionReply Helper::managefaces(const QVariantMap &args)
{
    int subaction = args.value("subaction").toInt();
    QString facesDir =
        KConfig(QString::fromLatin1(KDE_CONFDIR "/kdm/kdmrc"), KConfig::SimpleConfig)
            .group("X-*-Greeter").readEntry("FaceDir",
                QString(KStandardDirs::installPath("data") + "kdm/faces" + '/'));

    int code = 0;

    switch (subaction) {
    case CreateFacesDir:
        code = (createDir(facesDir) ? 0 : CreateFacesDirError);
        break;
    case RemoveFace:
        code = (removeFace(facesDir, args.value("user").toString())
                ? 0 : RemoveFaceError);
        break;
    case InstallFace:
        code = (installFace(facesDir,
                            args.value("user").toString(),
                            args.value("sourcefile").toString())
                ? 0 : InstallFaceError);
        break;
    default:
        return ActionReply::HelperError;
    }

    return createReply(code);
}

bool Helper::removeThemes(const QString &themesDir, QStringList &themes)
{
    QDir testDir(themesDir);
    if (!testDir.exists())
        return false;

    bool res = true;
    for (QStringList::iterator theme = themes.begin(); theme < themes.end(); ++theme) {
        QFileInfo info(*theme);
        // Security check
        if (info.absoluteDir() != QDir(themesDir) || !info.exists()) {
            res = false;
            continue;
        }

        if (removeDirTree(info.absoluteFilePath()))
            *theme = QString();
        else
            res = false;
    }

    return res;
}

bool Helper::installThemes(const QString &themesDir, QStringList &themes)
{
    QDir testDir(themesDir);
    if (!testDir.exists())
        return false;

    bool res = true;
    for (QStringList::iterator theme = themes.begin(); theme < themes.end(); ) {
        QFileInfo info(*theme);

        if (!info.exists()) {
            res = false;
            ++theme;
            continue;
        }

        if (installDirTree(info.absoluteFilePath(), themesDir + info.fileName() + '/')) {
            theme = themes.erase(theme);
        } else {
            res = false;
            ++theme;
        }
    }

    return res;
}

ActionReply Helper::managethemes(const QVariantMap &args)
{
    int subaction = args.value("subaction").toInt();
    QString themesDir = KStandardDirs::installPath("data") + "kdm/themes/";

    int code = 0;

    switch (subaction) {
    case CreateThemesDir:
        code = (createDir(themesDir) ? 0 : CreateThemesDirError);
        return createReply(code);
    case RemoveThemes: {
        QStringList themes = args.value("themes").toStringList();
        code = (removeThemes(themesDir, themes) ? 0 : RemoveThemesError);

        QVariantMap returnData;
        returnData["themes"] = themes;
        return createReply(code, &returnData);
    }
    case InstallThemes: {
        QStringList themes = args.value("themes").toStringList();
        code = (installThemes(themesDir, themes) ? 0 : InstallThemesError);

        QVariantMap returnData;
        returnData["failedthemes"] = themes;
        return createReply(code, &returnData);
    }
    default:
        return ActionReply::HelperError;
    }
}

KDE4_AUTH_HELPER_MAIN("org.kde.kcontrol.kcmkdm", Helper)
