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

    property bool show_multiple_batteries: false
    property bool show_remaining_time: false

    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', configChanged);
    }

    function configChanged() {
        show_multiple_batteries = plasmoid.readConfig("showMultipleBatteries");
        show_remaining_time = plasmoid.readConfig("showRemainingTime");
    }

    property Component compactRepresentation: Component {
        MouseArea {
            id: compactItem
            anchors.fill: parent
            hoverEnabled: true
            property int minimumWidth
            property int minimumHeight
            onClicked: plasmoid.togglePopup()

            property QtObject pmSource: plasmoid.rootItem.pmSource
            property bool hasBattery: pmSource.data["Battery"]["Has Battery"]
            property int percent: pmSource.data["Battery0"]["Percent"]
            property string batteryState: pmSource.data["Battery0"]["State"]
            property bool pluggedIn: pmSource.data["AC Adapter"]["Plugged in"]
            property bool showOverlay: false

            Component.onCompleted: {
                if (plasmoid.formFactor==Planar || plasmoid.formFactor==MediaCenter) {
                    minimumWidth = 32;
                    minimumHeight = 32;
                }
                plasmoid.addEventListener('ConfigChanged', configChanged);
            }

            function configChanged() {
                showOverlay = plasmoid.readConfig("showBatteryString");
            }

            function isConstrained() {
                return (plasmoid.formFactor == Vertical || plasmoid.formFactor == Horizontal);
            }

            Item {
                id: batteryContainer
                anchors.centerIn: parent
                property real size: Math.min(parent.width, parent.height)
                width: size
                height: size

                BatteryIcon {
                    id: batteryIcon
                    monochrome: true
                    hasBattery: compactItem.hasBattery
                    percent: compactItem.percent
                    pluggedIn: compactItem.pluggedIn
                    anchors.fill: parent
                }

                PlasmaCore.Theme { id: theme }

                Rectangle {
                    id: labelRect
                    // should be 40 when size is 90
                    width: Math.max(parent.size*4/9, 35)
                    height: width/2
                    anchors.centerIn: parent
                    color: theme.backgroundColor
                    border.color: "grey"
                    border.width: 2
                    radius: 4
                    opacity: hasBattery ? (showOverlay ? 0.5 : (isConstrained() ? 0 : compactItem.containsMouse*0.7)) : 0

                    Behavior on opacity { NumberAnimation { duration: 100 } }
                }

                Text {
                    id: overlayText
                    text: i18nc("overlay on the battery, needs to be really tiny", "%1%", percent);
                    color: theme.textColor
                    font.pixelSize: Math.max(batteryContainer.size/8, 11)
                    anchors.centerIn: labelRect
                    // keep the opacity 1 when labelRect.opacity=0.7
                    opacity: labelRect.opacity/0.7
                }
            }

            PlasmaCore.ToolTip {
                target: batteryContainer
                subText: {
                    var text="";
                    text += i18n("<b>Battery:</b>");
                    text += " ";
                    text += hasBattery ? plasmoid.rootItem.stringForState(batteryState, percent) : i18nc("Battery is not plugged in", "Not present");
                    text += "<br/>";
                    text += i18nc("tooltip", "<b>AC Adapter:</b>");
                    text += " ";
                    text += pluggedIn ? i18nc("tooltip", "Plugged in") : i18nc("tooltip", "Not plugged in");
                    return text;
                }
                image: "battery"
            }
        }
    }

    property QtObject pmSource: PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: sources
        onDataChanged: {
            var status = "PassiveStatus";
            if (data["Battery"]["Has Battery"]) {
                if (data["Battery0"]["Percent"] <= 10) {
                    status = "NeedsAttentionStatus";
                } else if (data["Battery0"]["State"] != "NoCharge") {
                    status = "ActiveStatus";
                }
            }
            plasmoid.status = status;
        }
    }

    function stringForState(state, percent) {
        if (state == "Charging")
            return i18n("%1% (charging)", percent);
        else if (state == "Discharging")
            return i18n("%1% (discharging)", percent);
        else
            return i18n("%1% (charged)", percent);

    }

    PopupDialog {
        id: dialogItem
        percent: pmSource.data["Battery0"]["Percent"]
        batteryState: pmSource.data["Battery0"]["State"]
        hasBattery: pmSource.data["Battery"]["Has Battery"]
        pluggedIn: pmSource.data["AC Adapter"]["Plugged in"]
        screenBrightness: pmSource.data["PowerDevil"]["Screen Brightness"]
        remainingMsec: parent.show_remaining_time ? Number(pmSource.data["Battery"]["Remaining msec"]) : 0
        showSuspendButton: pmSource.data["Sleep States"]["Suspend"]
        showHibernateButton: pmSource.data["Sleep States"]["Hibernate"]
        onSuspendClicked: {
            plasmoid.togglePopup();
            service = pmSource.serviceForSource("PowerDevil");
            var operationName = callForType(type);
            operation = service.operationDescription(operationName);
            service.startOperationCall(operation);
        }
        onBrightnessChanged: {
            service = pmSource.serviceForSource("PowerDevil");
            operation = service.operationDescription("setBrightness");
            operation.brightness = screenBrightness;
            service.startOperationCall(operation);
        }
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
        }

        function callForType(type) {
            if (type == ram) {
                return "suspendToRam";
            } else if (type == disk) {
                return "suspendToDisk";
            }

            return "suspendHybrid";
        }
    }
}
