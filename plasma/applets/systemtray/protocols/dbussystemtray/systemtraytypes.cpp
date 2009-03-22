/***************************************************************************
 *                                                                         *
 *   Copyright (C) 2009 Marco Martin <notmart@gmail.com>                   *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA .        *
 ***************************************************************************/

#include "systemtraytypes.h"


// Marshall the Icon data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const Icon &icon)
{
    argument.beginStructure();
    argument << icon.width;
    argument << icon.height;
    argument << icon.data;
    argument.endStructure();
    return argument;
}

// Retrieve the Icon data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, Icon &icon)
{
    qint32 width;
    qint32 height;
    QByteArray data;

    argument.beginStructure();
    argument >> width;
    argument >> height;
    argument >> data;
    argument.endStructure();

    icon.width = width;
    icon.height = height;
    icon.data = data;

    return argument;
}

// Marshall the IconVector data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const IconVector &iconVector)
{
    argument.beginArray(qMetaTypeId<Icon>());
    for (int i=0; i<iconVector.size(); ++i) {
        argument << iconVector[i]; 
    }
    argument.endArray();
    return argument;
}

// Retrieve the IconVector data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, IconVector &iconVector)
{
    argument.beginArray();
    iconVector.clear();

    while ( !argument.atEnd() ) {
       Icon element;
       argument >> element;
       iconVector.append(element);
    }

    argument.endArray();


    return argument;
}