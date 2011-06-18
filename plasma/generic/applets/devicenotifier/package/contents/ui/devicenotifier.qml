/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
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

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id: devicenotifier
    property bool removable
    property bool nonRemovable
    property bool all

    PlasmaCore.DataSource {
        id: dataSource
        engine: "hotplug"
        connectedSources: sources
        interval: 500
    }

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged)
    }

    function configChanged() {
        removable = plasmoid.readConfig("removableDevices")
        nonRemovable = plasmoid.readConfig("nonRemovableDevices")
        all = plasmoid.readConfig("allDevices")
    }

    Text {
        id: header
        // FIXME: The following "count" is not accessible
        // text: deviceView.deviceList.count>0 ? "Available Devices" : "No Devices Available"
        text: "Available Devices"
        anchors { top: parent.top; left: parent.left; right: parent.right }
        horizontalAlignment: Text.AlignHCenter
    }

    PlasmaWidgets.Separator {
        id: separator
        anchors { top: header.bottom; left: parent.left; right: parent.right }
        anchors { topMargin: 3 }
    }

    Column {
        id: deviceView
        anchors { top : separator.bottom; topMargin: 3 }
        Repeater {
            id: deviceList
            model: dataSource.sources
            DeviceItem {
                id: deviceItem
                width: devicenotifier.width
                deviceIcon: QIcon(dataSource.data[modelData]["icon"])
                deviceName: dataSource.data[modelData]["text"]
            }
        }
    }
}
