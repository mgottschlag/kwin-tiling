/*
 *   Copyright 2006 Aaron Seigo <aseigo@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License version 2 
 *   published by the Free Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <QApplication>
#include <QFile>

#include <KComponentData>
#include <KConfig>
#include <KConfigGroup>
#include <KDebug>
#include <KStandardDirs>

#include <KIO/Job>

void migrateAppletsrc()
{
    QString oldRc = KStandardDirs::locateLocal("config", "plasma-appletsrc");

    if (oldRc.isEmpty() || !QFile::exists(oldRc)) {
        //kDebug() << oldRc << "doesn't exist!";
        return;
    }

    QString newRc = KStandardDirs::locateLocal("config", "plasma-desktop-appletsrc");

    if (QFile::exists(newRc)) {
        //kDebug() << newRc << "exists!";
        return;
    }

    //kDebug() << "move" << oldRc << "to" << newRc;
    KIO::FileCopyJob *job = KIO::file_move(oldRc, newRc);
    job->exec();
}

void migratePlasmarc()
{
    QString oldRc = KStandardDirs::locateLocal("config", "plasmarc");

    if (oldRc.isEmpty() || !QFile::exists(oldRc)) {
        //kDebug() << oldRc << "doesn't exist!";
        return;
    }

    QString newRc = KStandardDirs::locateLocal("config", "plasma-desktoprc");

    if (QFile::exists(newRc)) {
        //kDebug() << newRc << "exists!";
        return;
    }

    KIO::FileCopyJob *job = KIO::file_copy(oldRc, newRc);
    job->exec();
    //kDebug() << "opening up" << oldRc << "for" << newRc;
    KConfig newConfig("plasma-desktoprc", KConfig::NoGlobals);

    foreach (const QString &group, newConfig.groupList()) {
        //kDebug() << group;
        if (group.startsWith("Theme") || group == "CachePolicies") {
            KConfigGroup newGroup(&newConfig, group);
            newGroup.deleteGroup();
        }
    }
}

int main(int argc, char *argv[])
{
    KComponentData cd("plasma-to-plasma-desktop");
    QApplication app(argc, argv);
    migratePlasmarc();
    migrateAppletsrc();

    return 0;
}

