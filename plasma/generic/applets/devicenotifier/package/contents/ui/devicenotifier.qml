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
    property bool removableDevices: true
    property bool nonRemovableDevices: false
    property bool allDevices: false

    PlasmaCore.DataSource {
        id: hpSource
        engine: "hotplug"
        connectedSources: []
        interval: 0
        onSourcesChanged: populateDevices()
    }

    PlasmaCore.DataSource {
        id: sdSource
        engine: "soliddevice"
        connectedSources: hpSource.sources
        interval: 0
        onSourcesChanged: populateDevices()
    }

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged);
        plasmoid.popupIcon = QIcon("device-notifier");
    }

    function configChanged() {
        removableDevices = plasmoid.readConfig("removableDevices");
        nonRemovableDevices = plasmoid.readConfig("nonRemovableDevices");
        allDevices = plasmoid.readConfig("allDevices");
        populateDevices();
    }

    function populateDevices() {
        var connected = hpSource.connectedSources;
        for (i=0; i<connected.length; i++)
            hpSource.disconnectSource(connected[i]);
        var sources = hpSource.sources;
        for (i=0; i<sources.length; i++) {
            if (removableDevices) { //Removable only
                if (!sdSource.data[sources[i]]["Removable"]) {
                    continue;
                }
            }
            else if (nonRemovableDevices) { //Non removable only
                if (sdSource.data[sources[i]]["Removable"]) {
                    continue;
                }
            }

            if (hpSource.connectedSources.indexOf(sources[i])<0) {
                hpSource.connectSource (sources[i]);
            }
        }
    }

    Text {
        id: header
        text: notifierDialog.model.length>0 ? "Available Devices" : "No Devices Available"
        anchors { top: parent.top; left: parent.left; right: parent.right }
        horizontalAlignment: Text.AlignHCenter
    }

    PlasmaWidgets.Separator {
        id: separator
        anchors { top: header.bottom; left: parent.left; right: parent.right }
        anchors { topMargin: 3 }
    }

    ListView {
        id: notifierDialog
        anchors {
            top : separator.bottom
            topMargin: 10
            bottom: devicenotifier.bottom
        }
        model: hpSource.connectedSources
        delegate: deviceItem
        highlight: deviceHighlighter
    }

    Component {
        id: deviceItem

        DeviceItem {
            width: devicenotifier.width
            udi: modelData
            icon: QIcon(hpSource.data[modelData]["icon"])
            deviceName: hpSource.data[modelData]["text"]
            percentUsage: {
                var freeSpace = Number(sdSource.data[modelData]["Free Space"]);
                var size = Number(sdSource.data[modelData]["Size"]);
                var used = size-freeSpace;
                return used*100/size;
            }
            mounted: true
            emblemIcon: QIcon(sdSource.data[modelData]["Emblems"][0])
            leftActionIcon: {
                var emblem = sdSource.data[modelData]["Emblems"][0];
                if (emblem == "emblem-mounted")
                    return QIcon("media-eject");
                else if (emblem == "emblem-unmounted")
                    return QIcon("emblem-mounted");
            }

            Component.onCompleted: {
                mounted = isMounted(modelData);
                if (mounted) {
                    operationName = "unmount";
                    //leftActionIcon = QIcon("media-eject");
                }
                else {
                    operationName = "mount";
                    //leftActionIcon = QIcon("emblem-mounted");
                }
                var actions = hpSource.data[modelData]["actions"];
                for (i=0; i<actions.length; i++)
                    print (actions[i]["icon"];
            }

            onLeftActionTriggered: {
                service = sdSource.serviceForSource (modelData);
                operation = service.operationDescription (operationName);
                service.startOperationCall (operation);
                mounted = isMounted(modelData)
                if (operationName=="mount" && mounted) {
                    operationName = "unmount";
                }
                else if (operationName=="unmount" && !mounted) {
                    operationName = "mount";
                }
            }
        }
    }

    Component {
        id: deviceHighlighter

        PlasmaCore.FrameSvgItem {
            width: devicenotifier.width
            imagePath: "widgets/frame"
            prefix: "raised"
            opacity: 0
            Behavior on opacity { NumberAnimation { duration: 150 } }
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
