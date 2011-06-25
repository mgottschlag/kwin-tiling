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
import org.kde.qtextracomponents 0.1

Item {
    id: deviceItem
    property string udi
    property string icon
    property alias deviceName: deviceLabel.text
    property string emblemIcon
    property alias leftActionIcon: leftAction.icon
    property bool mounted
    property bool expanded: (notifierDialog.currentExpanded==index)
    property alias percentUsage: freeSpaceBar.value
    signal leftActionTriggered

    property int deviceIconMargin: 10
    height: deviceIcon.height+(deviceIconMargin*2)+actionsList.height
    
    QIconItem {
        id: deviceIcon
        width: 32
        height: 32
        icon: QIcon(parent.icon)
        anchors {
            left: parent.left
            leftMargin: 10
            top: parent.top
            topMargin: deviceIconMargin
        }

        QIconItem {
            id: emblem
            width: 16
            height: 16
            icon: QIcon(emblemIcon)
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
            left: deviceLabel.left
        }
        text: {
            var n = hpSource.data[udi]["actions"].length;
            return n+" actions for this device";
        }
        font.italic: true
        font.pointSize: 8
        opacity: notifierDialog.currentIndex==index || expanded ? 1 : 0;

        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    PlasmaWidgets.Meter {
        id: freeSpaceBar
        height: 12
        anchors {
            top: deviceStatus.bottom
            topMargin: 0
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
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: freeSpaceBar.bottom
            bottomMargin: -actionsList.anchors.topMargin
        }
        hoverEnabled: true
        onEntered: {
            notifierDialog.currentIndex = index;
            notifierDialog.highlightItem.opacity = 1;
        }
        onExited: {
            notifierDialog.highlightItem.opacity = expanded ? 1 : 0;
            leftActionHighlight.opacity = 0;
            notifierDialog.currentIndex = -1;
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
            notifierDialog.currentExpanded = expanded ? -1 : index;
        }
    }

    ListView {
        id: actionsList
        anchors {
            top: freeSpaceBar.bottom
            topMargin: 10
            left: deviceIcon.right
            right: leftAction.right
        }
        model: hpSource.data[udi]["actions"];
        property int actionVerticalMargins: 4
        property int actionIconHeight: 30
        height: expanded ? ((actionIconHeight+(2*actionVerticalMargins))*model.length)+(spacing*(model.length-1))+anchors.topMargin : 0
        visible: expanded
        delegate: actionItem
        spacing: 2
        highlight: actionHighlighter
    }
    
    Component {
        id: actionItem
        
        ActionItem {
            icon: modelData["icon"]
            label: modelData["text"]
            predicate: modelData["predicate"]
        }
    }

    Component {
        id: actionHighlighter

        PlasmaCore.FrameSvgItem {
            width: actionsList.width
            imagePath: "widgets/frame"
            prefix: "raised"
            opacity: 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
    }

    function makeCurrent()
    {
        notifierDialog.currentIndex = index;
        notifierDialog.highlightItem.opacity=1;
    }
}
