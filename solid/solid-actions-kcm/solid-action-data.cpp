/***************************************************************************
 *   Copyright (C) 2009 by Ben Cooksley <ben@eclipse.endoftheinternet.org> *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA          *
 ***************************************************************************/

#include "solid-action-data.h"

#include <QList>

#include <KGlobal>
#include <kdesktopfileactions.h>
#include <KStandardDirs>
#include <KDesktopFile>
#include <KConfigGroup>

SolidActionData::SolidActionData()
{
    QStringList allPossibleDevices;

    // Fill the lists of possible device types / device values
    allPossibleDevices = KGlobal::dirs()->findAllResources("data", "solid/devices/");

    /// WARNING: We need to introspect first!

    // List all the known device actions, then add their name and all values to the appropriate lists
    foreach(const QString &desktop, allPossibleDevices) {
        KDesktopFile deviceFile(desktop);
        KConfigGroup deviceType = deviceFile.desktopGroup(); // Retrieve the configuration group where the user friendly name is
        types.insert(deviceType.readEntry("X-KDE-Solid-Actions-Type"), deviceType.readEntry("Name")); // Lets read the user friendly name
        QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(desktop, true); // Get the list of contained services
        QMap<QString,QString> deviceValues;
        foreach(const KServiceAction &deviceValue, services) { // We want every single action
            deviceValues.insert(deviceValue.name(), deviceValue.text()); // Add to the type - actions map
        }
        values.insert(deviceType.readEntry("X-KDE-Solid-Actions-Type"), deviceValues);
    }
}

QMap<QString, QString> SolidActionData::valueList(QString deviceType)
{
   return values.value(deviceType);
}

QString SolidActionData::generateUserString(QString className)
{
    QStringList splitString = className.split( QRegExp("(A-Z)"), QString::SkipEmptyParts );
    QString finalString;
    foreach( QString stringPiece, splitString ) {
        finalString += stringPiece + QChar(' ');
    }
    return finalString.trimmed();
}
