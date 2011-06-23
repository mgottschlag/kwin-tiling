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
import org.kde.qtextracomponents 0.1

Item {
    id: deviceItem
    property string udi
    property alias icon: deviceIcon.icon
    property alias deviceName: deviceLabel.text
    property alias emblemIcon: emblem.icon
    property alias leftActionIcon: leftAction.icon
    property string operationName
    property bool mounted
    property alias percentFreeSpace: freeSpaceBar.value
    signal leftActionTriggered

    property int deviceIconMargin: 10
    height: deviceIcon.height+(deviceIconMargin*2)
    
    QIconItem {
        id: deviceIcon
        width: 32
        height: 32
        anchors {
            left: parent.left
            leftMargin: 10
            top: parent.top
            topMargin: deviceIconMargin
            bottom: parent.bottom
            bottomMargin: deviceIconMargin
        }

        QIconItem {
            id: emblem
            width: 16
            height: 16
            anchors {
                left: parent.left
                leftMargin: 2
                bottom: parent.bottom
                bottomMargin: 2
            }
        }
    }


    Text {
        id: deviceLabel
        anchors {
            top: parent.top
            topMargin: 5
            left: deviceIcon.right
            leftMargin: 5
        }
    }

    Text {
        id: deviceStatus
        anchors {
            top: deviceLabel.bottom
            bottom: freeSpaceBar.top
            left: deviceLabel.left
        }
        text: "2 actions for this device"
        font.italic: true
        font.pointSize: 8
        opacity: 0

        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    PlasmaWidgets.Meter {
        id: freeSpaceBar
        height: 12
        anchors {
            bottom: parent.bottom
            bottomMargin: 5
            left: deviceLabel.left
            leftMargin: -2
            right: parent.right
            rightMargin: 35
        }
        opacity: mounted ? deviceStatus.opacity : 0
        minimum: 0
        maximum: 100
        meterType: PlasmaWidgets.Meter.BarMeterHorizontal
        svg: "widgets/bar_meter_horizontal"
    }

    QIconItem {
        id: leftAction
        width: 22
        height: 22
        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: deviceIcon.verticalCenter
        }
        Rectangle {
            id: leftActionHighlight
            color: "white"
            anchors.fill: parent
            opacity: 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            deviceStatus.opacity = 1;
            notifierDialog.currentIndex = index;
            notifierDialog.highlightItem.opacity = 1;
        }
        onExited: {
            deviceStatus.opacity = 0;
            notifierDialog.highlightItem.opacity = 0;
            leftActionHighlight.opacity = 0;
        }
        onPositionChanged: {
            if (mouse.x>=leftAction.x && mouse.x<=leftAction.x+leftAction.width
             && mouse.y>=leftAction.y && mouse.y<=leftAction.y+leftAction.height)
            {
                leftActionHighlight.opacity = 0.3;
            }
            else {
                leftActionHighlight.opacity = 0;
            }
        }
        onClicked: {
            if (mouse.x>=leftAction.x && mouse.x<=leftAction.x+leftAction.width
             && mouse.y>=leftAction.y && mouse.y<=leftAction.y+leftAction.height)
            {
                leftActionTriggered();
            }
        }
    }

}
