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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id: devicenotifier
    property int minimumWidth: 290
    property int minimumHeight: 340
    property string devicesType: "removable"
    property string expandedDevice

    PlasmaCore.DataSource {
        id: hpSource
        engine: "hotplug"
        connectedSources: sources
        interval: 0
    }

    PlasmaCore.DataSource {
        id: sdSource
        engine: "soliddevice"
        connectedSources: hpSource.sources
        interval: 0
        property string last
        onSourceAdded: {
            last = source;
        }
        onDataChanged: {
            if (last != "") {
                if (devicesType == "all" ||
                    (devicesType == "removable" && data[last]["Removable"] == true) ||
                    (devicesType == "nonRemovable" && data[last]["Removable"] == false)) {
                    expandDevice(last)
                }
            }
        }
    }

    PlasmaCore.DataSource {
        id: statusSource
        engine: "devicenotifications"
        property string last
        onSourceAdded: {
            last = source;
            connectSource(source);
        }
        onDataChanged: {
            if (last!="")
                statusBar.setData(data[last]["error"], data[last]["errorDetails"], data[last]["udi"]);
        }
    }

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged);
    }

    function configChanged() {
        var all = plasmoid.readConfig("allDevices");
        var removable = plasmoid.readConfig("removableDevices");
        if (all == true) {
            devicesType = "all";
            filterModel.filterRegExp = "";
        } else if (removable == true) {
            devicesType = "removable";
            filterModel.filterRegExp = "true";
        } else {
            devicesType = "nonRemovable";
            filterModel.filterRegExp = "false";
        }
        notifierDialog.currentIndex = -1;
        notifierDialog.currentExpanded = -1;
    }

    function expandDevice(udi)
    {
        expandedDevice = udi
        setPopupIcon ("preferences-desktop-notification", 7500);
        plasmoid.status = "NeedsAttentionStatus";
        plasmoid.showPopup()
    }

    function setPopupIcon (icon, timeout) {
        plasmoid.setPopupIconByName(icon);
        popupIconTimer.interval = timeout;
        popupIconTimer.restart();
    }


    Timer {
        id: popupIconTimer
        onTriggered: plasmoid.setPopupIconByName("device-notifier");
    }

    Text {
        id: header
        text: filterModel.count>0 ? "Available Devices" : "No Devices Available"
        anchors { top: parent.top; topMargin: 3; left: parent.left; right: parent.right }
        horizontalAlignment: Text.AlignHCenter
    }

    PlasmaWidgets.Separator {
        id: headerSeparator
        anchors { top: header.bottom; left: parent.left; right: parent.right }
        anchors { topMargin: 3 }
    }

    ListView {
        id: notifierDialog
        anchors {
            top : headerSeparator.bottom
            topMargin: 10
            bottom: statusBarSeparator.top
            left: parent.left
            right: parent.right
        }
        model: PlasmaCore.SortFilterModel {
            id: filterModel
            sourceModel: PlasmaCore.DataModel {
                dataSource: sdSource
            }
            filterRole: "Removable"
            filterRegExp: "true"
            sortRole: "Removable"
            sortOrder: Qt.DescendingOrder
        }
        delegate: deviceItem
        highlight: deviceHighlighter
        highlightMoveDuration: 250
        highlightMoveSpeed: 1
        clip: true

        property int currentExpanded: -1
        Component.onCompleted: currentIndex=-1
    }

    Component {
        id: deviceItem

        DeviceItem {
            id: wrapper
            width: devicenotifier.width
            udi: DataEngineSource
            icon: hpSource.data[udi]["icon"]
            deviceName: hpSource.data[udi]["text"]
            emblemIcon: Emblems[0]
            state: model["State"]

            percentUsage: {
                var freeSpace = Number(model["Free Space"]);
                var size = Number(model["Size"]);
                var used = size-freeSpace;
                return used*100/size;
            }
            leftActionIcon: {
                if (emblemIcon == "emblem-mounted") {
                    return QIcon("media-eject");
                } else if (emblemIcon == "emblem-unmounted") {
                    return QIcon("emblem-mounted");
                }
                else return QIcon("");
            }
            mounted: emblemIcon=="emblem-mounted"

            onLeftActionTriggered: {
                operationName = mounted ? "unmount" : "mount";
                service = sdSource.serviceForSource(udi);
                operation = service.operationDescription(operationName);
                service.startOperationCall(operation);
                setPopupIcon("dialog-ok", 2500);
            }
            property bool isLast: (expandedDevice == udi)
            onIsLastChanged: {
                if (isLast) {
                    notifierDialog.currentExpanded = index
                }
            }
        }
    }

    Component {
        id: deviceHighlighter

        PlasmaCore.FrameSvgItem {
            width: devicenotifier.width
            imagePath: "widgets/viewitem"
            prefix: "hover"
            opacity: 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
        }
    }

    PlasmaWidgets.Separator {
        id: statusBarSeparator
        anchors {
            bottom: statusBar.top
            bottomMargin: statusBar.visible ? 3:0
            left: parent.left
            right: parent.right
        }
        visible: statusBar.height>0
    }

    StatusBar {
        id: statusBar
        anchors {
            left: parent.left
            leftMargin: 5
            right: parent.right
            rightMargin: 5
            bottom: parent.bottom
            bottomMargin: 5
        }
    }

    function isMounted (udi) {
        var types = sdSource.data[udi]["Device Types"];
        if (types.indexOf("Storage Access")>=0) {
            if (sdSource.data[udi]["Accessible"]) {
                return true;
            }
            else {
                return false;
            }
        }
        else if (types.indexOf("Storage Volume")>=0 && types.indexOf("OpticalDisc")>=0) {
            return true;
        }
        else {
            return false;
        }
    }
}
