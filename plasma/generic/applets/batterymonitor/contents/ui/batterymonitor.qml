/*
 *   Copyright 2011 Sebastian Kügler <sebas@kde.org>
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: batterymonitor
    property int minimumWidth: dialogItem.width
    property int minimumHeight: dialogItem.height

    property bool show_charge: false
    property bool show_multiple_batteries: false
    property bool show_remaining_time: false

    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', configChanged);
    }

    function configChanged() {
        show_charge = plasmoid.readConfig("showBatteryString");
        show_multiple_batteries = plasmoid.readConfig("showMultipleBatteries");
        show_remaining_time = plasmoid.readConfig("showRemainingTime");
    }

    property Component compactRepresentation: Component {
        MouseArea {
            id: mouseArea
            anchors.fill:parent
            onClicked: plasmoid.togglePopup()

            PlasmaCore.Svg{
                id: iconSvg
                imagePath: "icons/battery"
            }

            property QtObject pmSource: plasmoid.rootItem.pmSource

            Item {
                anchors.centerIn: parent
                width: Math.min(parent.width, parent.height)
                height: width
                PlasmaCore.SvgItem {
                    anchors.fill: parent
                    svg: iconSvg
                    elementId: "Battery"
                }

                PlasmaCore.SvgItem {
                    anchors.fill: parent
                    svg: iconSvg
                    elementId: pmSource.data["Battery"]["Has Battery"] ? parent.fillElement(pmSource.data["Battery0"]["Percent"]) : "Unavailable"
                }

                function fillElement(p) {
                    if (p > 95) {
                        return "Fill100";
                    } else if (p > 80) {
                        return "Fill80";
                    } else if (p > 50) {
                        return "Fill60";
                    } else if (p > 20) {
                        return "Fill40";
                    } else if (p > 10) {
                        return "Fill20";
                    }
                    return "";
                }

                PlasmaCore.SvgItem {
                    anchors.fill: parent
                    svg: iconSvg
                    elementId: pmSource.data["AC Adapter"]["Plugged in"] ? "AcAdapter" : ""
                }

                Rectangle {
                    id: chargeInfo
                    width: percent.paintedWidth+4    // 4 = left/right margins
                    height: percent.paintedHeight+4  // 4 = top/bottom margins
                    anchors.centerIn: parent
                    color: "white"
                    border.color: "grey"
                    border.width: 2
                    radius: 3
                    visible: plasmoid.rootItem.show_charge && pmSource.data["Battery"]["Has Battery"]
                    opacity: 0.7

                    Text {
                        id: percent
                        text: i18nc("overlay on the battery, needs to be really tiny", "%1%", pmSource.data["Battery0"]["Percent"]);
                        font.bold: true
                        anchors.centerIn: parent
                        visible: parent.visible
                    }
                }
            }
        }
    }

    property QtObject pmSource: PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["AC Adapter", "Battery", "Battery0", "PowerDevil"]
        interval: 0
    }

    PopupDialog {
        id: dialogItem
        percent: pmSource.data["Battery0"]["Percent"]
        pluggedIn: pmSource.data["AC Adapter"]["Plugged in"]
        screenBrightness: pmSource.data["PowerDevil"]["Screen Brightness"]
<<<<<<< HEAD
        remainingMsec: Number(pmSource.data["Battery"]["Remaining msec"])
        showRemainingTime: parent.show_remaining_time
=======
        remainingMsec: pmSource.data["Battery"]["Remaining msec"]
>>>>>>> d34784102ef6d71fb115fc438680b6dddaf30414
        onSleepClicked: {
            dialog.visible=false
            service = pmSource.serviceForSource("PowerDevil");
            operation = service.operationDescription("suspendToRam");
            service.startOperationCall(operation);
        }
        onHibernateClicked: {
            dialog.visible=false
            service = pmSource.serviceForSource("PowerDevil");
            operation = service.operationDescription("suspendToDisk");
            service.startOperationCall(operation);
        }
        onBrightnessChanged: {
            service = pmSource.serviceForSource("PowerDevil");
            operation = service.operationDescription("setBrightness");
            operation.brightness = screenBrightness;
            service.startOperationCall(operation);
        }
<<<<<<< HEAD
        property int cookie1: -1
        property int cookie2: -1
        onPowermanagementChanged: {
            service = pmSource.serviceForSource("PowerDevil");
            if (checked) {
                var op1 = service.operationDescription("stopSuppressingSleep");
                op1.cookie = cookie1;
                var op2 = service.operationDescription("stopSuppressingScreenPowerManagement");
                op2.cookie = cookie2;

                var job1 = service.startOperationCall(op1);
                job1.finished.connect(function(job) {
                    cookie1 = -1;
                });

                var job2 = service.startOperationCall(op2);
                job1.finished.connect(function(job) {
                    cookie2 = -1;
                });
            } else {
                var reason = i18n("The battery applet has enabled system-wide inhibition");
                var op1 = service.operationDescription("beginSuppressingSleep");
                op1.reason = reason;
                var op2 = service.operationDescription("beginSuppressingScreenPowerManagement");
                op2.reason = reason;

                var job1 = service.startOperationCall(op1);
                job1.finished.connect(function(job) {
                    cookie1 = job.result;
                });

                var job2 = service.startOperationCall(op2);
                job1.finished.connect(function(job) {
                    cookie2 = job.result;
                });
            }
=======
        onPowermanagementChanged: {
            // TODO
>>>>>>> d34784102ef6d71fb115fc438680b6dddaf30414
        }
    }
}
