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
    property alias icon: deviceIcon.icon
    property alias deviceName: deviceLabel.text
    property alias emblemIcon: emblem.icon
    property alias leftActionIcon: leftAction.icon
    property string operationName
    property alias percentFreeSpace: deviceMeter.value
    signal leftActionTriggered

    height: deviceIcon.height

    PlasmaWidgets.IconWidget {
        id: deviceIcon
        anchors.fill: parent
        drawBackground: true
        orientation: QtHorizontal
        width: parent.width
        maximumIconSize: "32x32"
        MouseArea{
            anchors.fill: parent
            hoverEnabled: true
            onEntered: { deviceMeter.visible=true; print("enter");}
            onExited: deviceMeter.visible=false;
        }
    }
    PlasmaWidgets.IconWidget {
        id: emblem
        anchors {
            left: parent.left
            leftMargin: 12
            bottom: parent.bottom
            bottomMargin: 12
        }
        width: 16; height: 16
    }
    Text {
        id: deviceLabel
        anchors {
            top: parent.top
            topMargin: 5
            left: deviceIcon.right
            leftMargin: -3
        }
    }
    PlasmaWidgets.Meter {
        id: deviceMeter
        minimum: 0
        maximum: 100
        anchors {
            bottom: parent.bottom
            bottomMargin: 5
            left: deviceIcon.right
            leftMargin: -5
            right: parent.right
            rightMargin: 35
        }
        height: 12
        //visible: value>0
        meterType: PlasmaWidgets.Meter.BarMeterHorizontal
        svg: "widgets/bar_meter_horizontal"
    }
    PlasmaWidgets.IconWidget {
        id: leftAction
        width: 22
        height: 22
        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: deviceIcon.verticalCenter
        }
    }
}
