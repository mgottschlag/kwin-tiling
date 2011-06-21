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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id: deviceDelegate
    property string udi
    property alias deviceIcon: device.icon
    property alias deviceName: device.text
    property alias leftActionIcon: leftAction.icon
    property string operationName
    property alias percentFreeSpace: deviceMeter.value
    signal leftActionTriggered

    height: device.height

    Item {
        id: leftActionContainer
        //z: deviceContainer.z+1
        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: deviceContainer.verticalCenter
        }
        width: leftAction.width
        height: leftAction.height
        PlasmaWidgets.IconWidget {
            id: leftAction
            width: 22
            height: 22
        }
        MouseArea {
            anchors.fill: parent
            onClicked: leftActionTriggered()
        }
    }

    Item {
        id: deviceContainer
        width: parent.width
        height: device.height
        anchors { top: parent.top; left: parent.left }
        PlasmaWidgets.IconWidget {
            id: device
            anchors.fill: parent
            drawBackground: true
            orientation: QtHorizontal
            width: parent.width
        }
    }

    Item {
        id: meterContainer
        anchors { bottom: deviceContainer.bottom; bottomMargin: 5; left: deviceContainer.left; leftMargin: 45; right: deviceContainer.right; rightMargin: 35 }
        height: 12
        PlasmaWidgets.Meter {
            id: deviceMeter
            minimum: 0
            maximum: 100
            //visible: value>0
            meterType: PlasmaWidgets.Meter.BarMeterHorizontal
            anchors.fill: parent
            svg: "widgets/bar_meter_horizontal"
        }
    }

    MouseArea{}
}
