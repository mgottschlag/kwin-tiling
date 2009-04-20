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


// Marshall the ImageStruct data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const ImageStruct &icon)
{
    argument.beginStructure();
    argument << icon.width;
    argument << icon.height;
    argument << icon.data;
    argument.endStructure();
    return argument;
}
#include <KDebug>

// Retrieve the ImageStruct data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, ImageStruct &icon)
{
    qint32 width;
    qint32 height;
    QByteArray data;

    argument.beginStructure();
    kDebug() << "begun structure";
    argument >> width;
    kDebug() << width;
    argument >> height;
    kDebug() << height;
    argument >> data;
    kDebug() << data.size();
    argument.endStructure();

    icon.width = width;
    icon.height = height;
    icon.data = data;

    return argument;
}

// Marshall the ImageVector data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const ImageVector &iconVector)
{
    argument.beginArray(qMetaTypeId<ImageStruct>());
    for (int i=0; i<iconVector.size(); ++i) {
        argument << iconVector[i]; 
    }
    argument.endArray();
    return argument;
}

// Retrieve the ImageVector data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, ImageVector &iconVector)
{
    argument.beginArray();
    iconVector.clear();

    while ( !argument.atEnd() ) {
       ImageStruct element;
       argument >> element;
       iconVector.append(element);
    }

    argument.endArray();
    return argument;
}

// Marshall the ToolTipStruct data into a D-BUS argument
const QDBusArgument &operator<<(QDBusArgument &argument, const ToolTipStruct &toolTip)
{
    argument.beginStructure();
    argument << toolTip.icon;
    argument << toolTip.image;
    argument << toolTip.title;
    argument << toolTip.subTitle;
    argument.endStructure();
    return argument;
}

// Retrieve the ToolTipStruct data from the D-BUS argument
const QDBusArgument &operator>>(const QDBusArgument &argument, ToolTipStruct &toolTip)
{
    QString icon;
    ImageVector image;
    QString title;
    QString subTitle;

    argument.beginStructure();
    argument >> icon;
    argument >> image;
    argument >> title;
    argument >> subTitle;
    argument.endStructure();

    toolTip.icon = icon;
    toolTip.image = image;
    toolTip.title = title;
    toolTip.subTitle = subTitle;

    return argument;
}
