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
    property int state
    property alias leftActionIcon: leftAction.icon
    property bool mounted
    property bool expanded: (notifierDialog.currentExpanded==index)
    property alias percentUsage: freeSpaceBar.value
    signal leftActionTriggered

    property int deviceIconMargin: 10
    height: deviceIcon.height+(deviceIconMargin*2)+actionsList.height

    // FIXME: Device item loses focus on mounting/unmounting it,
    // or specifically, when some UI element changes.
    QIconItem {
        id: deviceIcon
        width: 32
        height: 32
        z: 900
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

    PlasmaCore.Theme { id: theme }

    Text {
        id: deviceStatus
        anchors {
            top: deviceLabel.bottom
            left: deviceLabel.left
        }
        // FIXME: state changes do not reach the plasmoid if the
        // device was already attached when the plasmoid was
        // initialized
        text: parent.state==0 ? idleStatus() : (parent.state==1 ? i18nc("Accessing is a less technical word for Mounting; translation should be short and mean \'Currently mounting this device\'", "Accessing...") : i18nc("Removing is a less technical word for Unmounting; translation shoud be short and mean \'Currently unmounting this device\'", "Removing..."))
        font.italic: true
        font.pointSize: 8
        color: "#99"+(theme.textColor.toString().substr(1))
        opacity: mouseArea.containsMouse || expanded ? 1 : 0;

        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    function idleStatus() {
        var actions = hpSource.data[udi]["actions"];
        if (actions.length > 1) {
            return i18n("%1 actions for this device", actions.length);
        } else {
            return actions[0]["text"];
        }
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

    Item {
        id: freeSpaceBarPlaceholder
        anchors.fill: freeSpaceBar
        z: 900
    }

    PlasmaCore.ToolTip {
        target: freeSpaceBarPlaceholder
        subText: i18nc("@info:status Free disk space", "%1 free", model["Free Space Text"])
    }

    QIconItem {
        id: leftAction
        width: 22
        height: 22
        z: 900
        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: deviceIcon.verticalCenter
        }
        visible: !busySpinner.visible
    }

    PlasmaCore.ToolTip {
        target: leftAction
        subText: {
            if (!model["Accessible"]) {
                return i18n("Click to mount this device.")
            } else if (model["Device Types"].indexOf("Optical Disk") != -1) {
                return i18n("Click to eject this disc.")
            } else if (model["Removable"]) {
                return i18n("Click to safely remove this device.")
            } else {
                return i18n("Click to access this device from other applications.")
            }
        }
    }

    PlasmaCore.ToolTip {
        target: deviceIcon
        subText: {
            if (model["Accessible"]) {
                if (model["Removable"]) {
                    return i18n("It is currently <b>not safe</b> to remove this device: applications may be accessing it. Click the eject button to safely remove this device.")
                } else {
                    return i18n("This device is currently accessible.")
                }
            } else {
                if (model["Removable"]) {
                     if (model["In Use"]) {
                        return i18n("It is currently <b>not safe</b> to remove this device: applications may be accessing other volumes on this device. Click the eject button on these other volumes to safely remove this device.");
                    } else {
                        return i18n("It is currently safe to remove this device.")
                    }
                } else {
                    return i18n("This device is not currently accessible.")
                }
            }
        }
    }

    PlasmaWidgets.BusyWidget {
        id: busySpinner
        width: 22
        height: 22
        anchors {
            right: parent.right
            rightMargin: 10
            verticalCenter: deviceIcon.verticalCenter
        }
        running: visible
        visible: state!=0
    }

    MouseArea {
        id: mouseArea
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: freeSpaceBar.bottom
            // to remove the gap between device items
            bottomMargin: -actionsList.anchors.topMargin
        }
        hoverEnabled: true
        onEntered: {
            notifierDialog.currentIndex = index;
            notifierDialog.highlightItem.opacity = 1;
        }
        onExited: {
            notifierDialog.highlightItem.opacity = expanded ? 1 : 0;
        }
        onClicked: {
            if (leftAction.visible
             && mouse.x>=leftAction.x && mouse.x<=leftAction.x+leftAction.width
             && mouse.y>=leftAction.y && mouse.y<=leftAction.y+leftAction.height)
            {
                leftActionTriggered();
            }
            else {
                var actions = hpSource.data[udi]["actions"];
                if (actions.length==1) {
                    service = hpSource.serviceForSource(udi);
                    operation = service.operationDescription("invokeAction");
                    operation.predicate = actions[0]["predicate"];
                    service.startOperationCall(operation);
                } else {
                    notifierDialog.currentExpanded = expanded ? -1 : index;
                }
            }
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
        interactive: false
        model: hpSource.data[udi]["actions"]
        property int actionVerticalMargins: 5
        property int actionIconHeight: 30
        height: expanded ? ((actionIconHeight+(2*actionVerticalMargins))*model.length)+anchors.topMargin : 0
        opacity: expanded ? 1 : 0
        delegate: actionItem
        highlight: actionHighlighter
        Behavior on opacity { NumberAnimation { duration: 150 } }
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
            imagePath: "widgets/viewitem"
            prefix: "hover"
            opacity: 0
            Behavior on opacity {
                NumberAnimation {
                    duration: 250
                    easing.type: Easing.OutQuad
                }
            }
        }
    }

    function makeCurrent()
    {
        notifierDialog.currentIndex = index;
        notifierDialog.highlightItem.opacity=1;
    }
}
