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
#include <QMetaProperty>

#include <KGlobal>
#include <kdesktopfileactions.h>
#include <KStandardDirs>
#include <KStringHandler>
#include <KDesktopFile>
#include <KConfigGroup>
#include <KDebug>

#include <Solid/AcAdapter>
#include <Solid/AudioInterface>
#include <Solid/Battery>
#include <Solid/Block>
#include <Solid/Button>
#include <Solid/Camera>
#include <Solid/DvbInterface>
#include <Solid/GenericInterface>
#include <Solid/NetworkInterface>
#include <Solid/PortableMediaPlayer>
#include <Solid/Processor>
#include <Solid/StorageAccess>
#include <Solid/StorageDrive>
#include <Solid/OpticalDrive>
#include <Solid/StorageVolume>
#include <Solid/OpticalDisc>

SolidActionData::SolidActionData()
{
    QStringList allPossibleDevices;

    QList<QMetaObject> interfaceList = fillInterfaceList();
    foreach( QMetaObject interface, interfaceList ) {
        QString ifaceName = interface.className();
        ifaceName.remove(0, ifaceName.lastIndexOf(':') + 1);
        types.insert(ifaceName, generateUserString(ifaceName));
        QMap<QString,QString> deviceValues;
        for( int doneProps = 0; interface.propertyCount() > doneProps; doneProps = doneProps + 1 ) {
            QMetaProperty ifaceProp = interface.property(doneProps);
            deviceValues.insert(ifaceProp.name(), generateUserString(ifaceProp.name()));
        }
        values.insert(ifaceName, deviceValues);
    }

    // Fill the lists of possible device types / device values
    allPossibleDevices = KGlobal::dirs()->findAllResources("data", "solid/devices/");
    // List all the known device actions, then add their name and all values to the appropriate lists
    foreach(const QString &desktop, allPossibleDevices) {
        KDesktopFile deviceFile(desktop);
        KConfigGroup deviceType = deviceFile.desktopGroup(); // Retrieve the configuration group where the user friendly name is
        types.insert(deviceType.readEntry("X-KDE-Solid-Actions-Type"), deviceType.readEntry("Name")); // Lets read the user friendly name
        QList<KServiceAction> services = KDesktopFileActions::userDefinedServices(desktop, true); // Get the list of contained services
        QMap<QString,QString> deviceValues = values.value(deviceType.readEntry("X-KDE-Solid-Actions-Type"));
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
    QString finalString;
    QRegExp camelCase("([A-Z])"); // Create the split regexp

    finalString = className.remove(0, className.lastIndexOf(':') + 1); // Remove any Class information
    finalString = finalString.replace( camelCase, " \\1" ); // Use Camel Casing to add spaces
    finalString = KStringHandler::capwords( finalString ); // Captialise everything
    return finalString.trimmed();
}

QList<QMetaObject> SolidActionData::fillInterfaceList()
{
    QList<QMetaObject> interfaces;
    interfaces.append( Solid::AcAdapter::staticMetaObject );
    interfaces.append( Solid::AudioInterface::staticMetaObject );
    interfaces.append( Solid::Battery::staticMetaObject );
    interfaces.append( Solid::Block::staticMetaObject );
    interfaces.append( Solid::Button::staticMetaObject );
    interfaces.append( Solid::Camera::staticMetaObject );
    interfaces.append( Solid::DvbInterface::staticMetaObject );
    interfaces.append( Solid::GenericInterface::staticMetaObject );
    interfaces.append( Solid::NetworkInterface::staticMetaObject );
    interfaces.append( Solid::PortableMediaPlayer::staticMetaObject );
    interfaces.append( Solid::Processor::staticMetaObject );
    //interfaces.append( Solid::SerialInterface::staticMetaObject ); // The header does not exist?
    interfaces.append( Solid::StorageAccess::staticMetaObject );
    interfaces.append( Solid::StorageDrive::staticMetaObject );
    interfaces.append( Solid::OpticalDrive::staticMetaObject );
    interfaces.append( Solid::StorageVolume::staticMetaObject );
    interfaces.append( Solid::OpticalDisc::staticMetaObject );
    //interfaces.append( Solid::Video::staticMetaObject ); // The header does not exist?
    return interfaces;
}
