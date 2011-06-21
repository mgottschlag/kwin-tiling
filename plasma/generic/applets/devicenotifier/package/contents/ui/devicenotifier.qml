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
    property int deviceTypeConfig: 0

    PlasmaCore.DataSource {
        id: hpSource
        engine: "hotplug"
        connectedSources: sources
        interval: 0
        /*onSourcesChanged: {
            for (i=0; i<sources.length; i++) {
                if (sdSource.data[sources[i]]["Removable"]) connectSource(sources[i]);
            }
        }*/
    }

    PlasmaCore.DataSource {
        id: sdSource
        engine: "soliddevice"
        connectedSources: hpSource.sources
        interval: 0
    }

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged);
        plasmoid.popupIcon = QIcon("device-notifier");
    }

    function configChanged() {
        if (plasmoid.readConfig("removableDevices"))
            deviceTypeConfig = 0;
        else if (plasmoid.readConfig("nonRemovableDevices"))
            deviceTypeConfig = 1;
        else if(plasmoid.readConfig("allDevices"))
            deviceTypeConfig = 2;
        else
            deviceTypeConfig = -1;
    }

    Text {
        id: header
        text: deviceList.count>0 ? "Available Devices" : "No Devices Available"
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
            model: hpSource.sources
            DeviceItem {
                id: deviceItem
                width: devicenotifier.width
                udi: modelData
                deviceIcon: QIcon(hpSource.data[modelData]["icon"])
                deviceName: hpSource.data[modelData]["text"]
                percentFreeSpace: sdSource.data[modelData]["File Path"]=="" ? 0 : Number(sdSource.data[modelData]["Free Space"])*100/Number(sdSource.data[modelData]["Size"])
                Component.onCompleted: {
                    //var fs = Number(sdSource.data[modelData]["Size"]);
                    //print (modelData+": "+fs.toString());
                    var types = sdSource.data[modelData]["Device Types"];
                    if (types.indexOf("Storage Access")>=0) {
                        if (sdSource.data[modelData]["Accessible"]) {
                            operationName = "unmount";
                            leftActionIcon = QIcon("media-eject");
                        }
                        else {
                            operationName = "mount";
                            leftActionIcon = QIcon("emblem-mounted");
                        }
                    }
                    else if (types.indexOf("Storage Volume")>=0 && types.indexOf("OpticalDisc")>=0) {
                        operationName = "unmount";
                        leftActionIcon = QIcon("media-eject");
                    }
                }
                onLeftActionTriggered: {
                    service = sdSource.serviceForSource (modelData);
                    operation = service.operationDescription (operationName);
                    service.startOperationCall (operation);
                    if (operationName=="mount") {
                        operationName = "unmount";
                        leftActionIcon = QIcon("media-eject");
                    }
                    else if (operationName=="unmount") {
                        operationName = "mount";
                        leftActionIcon = QIcon("emblem-mounted");
                    }
                }
            }
        }
    }
}
