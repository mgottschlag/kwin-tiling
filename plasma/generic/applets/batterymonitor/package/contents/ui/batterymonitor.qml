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

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import "core"

Item {
    id: batterymonitor
    width: 48
    height: 48

    PlasmaCore.DataSource {
        id: pmSource
        engine: "powermanagement"
        connectedSources: ["AC Adapter", "Battery", "Battery0", "PowerDevil"]
        interval: 0
    }

    PlasmaCore.Dialog {
        id: dialog
        mainItem: PopupDialog {
            id: dialogItem
            percent: pmSource.data["Battery0"]["Percent"]
            pluggedIn: pmSource.data["AC Adapter"]["Plugged in"]
            screenBrightness: pmSource.data["PowerDevil"]["Screen brightness"]
            onSleepClicked: {
                dialog.visible=false
                service = pmSource.serviceForSource("PowerDevil");
                operation = service.operationDescription("suspend");
                service.startOperationCall(operation);
            }
            onHibernateClicked: {
                dialog.visible=false
                service = pmSource.serviceForSource("PowerDevil");
                operation = service.operationDescription("hibernate");
                service.startOperationCall(operation);
            }
            onBrightnessChanged: {
                service = pmSource.serviceForSource("PowerDevil");
                operation = service.operationDescription("setBrightness");
                operation.brightness = screenBrightness;
                service.startOperationCall(operation);
            }
            onProfileChanged: {
                service = pmSource.serviceForSource("PowerDevil");
                operation = service.operationDescription("setProfile");
                operation.profile = currentProfile;
                service.startOperationCall(operation);
            }
            Component.onCompleted: {
                var profiles = pmSource.data["PowerDevil"]["Available profiles"];
                for (var i in profiles) {
                    print(i);
                    print(profiles[i]);
                }
            }
        }
        Component.onCompleted: {
            setAttribute(Qt.WA_X11NetWmWindowTypeDock, true);
        }
    }

    MouseArea {
        anchors.fill: parent
        onClicked: {
            dialog.visible=!dialog.visible
        }
    }

    PlasmaCore.Svg{
        id: iconSvg
        imagePath: "icons/battery"
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent
        svg: iconSvg
        elementId: "Battery"
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent
        svg: iconSvg
        elementId: fillElement(pmSource.data["Battery0"]["Percent"]) 
    }

    function fillElement(p) {
        if (p >= 100) {
            return "Fill100";
        } else if (p > 80) {
            return "Fill80";
        } else if (p > 60) {
            return "Fill60";
        } else if (p > 40) {
            return "Fill40";
        } else if (p > 20) {
            return "Fill20";
        }
        return "";
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent
        svg: iconSvg
        elementId: pmSource.data["AC Adapter"]["Plugged in"] ? "AcAdapter" : ""
    }
}
