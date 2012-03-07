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
import org.kde.plasma.components 0.1 as Components
import org.kde.qtextracomponents 0.1

Item {
    id: dialog
    width: 400
    height: 170

    property int percent
    property bool pluggedIn
    property alias screenBrightness: brightnessSlider.value
    //property alias currentProfileIndex: profiles.currentIndex

    signal sleepClicked
    signal hibernateClicked
    signal brightnessChanged(int screenBrightness)
    //signal profileChanged(string profile)

    /*function addProfile(profile)    { profiles.addItem(profile);        }
    function setProfile (index)     { profiles.currentIndex = index;    }
    function clearProfiles()        { profiles.clear();                 }*/

    Column {
        id: values

        Row {
            Components.Label {
                id: batteryLabel
                text: i18n("Battery:")
            }
            Components.Label {
                id: batteryValue
                text: {
                    if (percent == 0) {
                        return "Not present";
                    }
                    var txt=percent+"% (";
                    if (percent<100) {
                        if (pluggedIn) txt += "charging";
                        else txt += "discharging";
                    } else {
                        txt += "charged";
                    }
                    txt += ")"
                    return txt;
                }
                font.bold: true
            }
        }

        Components.Slider {
            id: brightnessSlider
            minimumValue: 0
            maximumValue: 100
            stepSize: 10
            anchors {
                left: parent.left
                top: batteryValue.bottom
                topMargin: 6
                right: parent.right
            }
            onValueChanged: brightnessChanged(value)
        }

        Row {
            Components.Label {
                id: profileLabel
                text: i18n("Enabled:")
                horizontalAlignment: Text.AlignRight
            }
            // TODO: what to do on check/uncheck?
            Components.Switch {
                id: profiles
                checked: true
            }
        }

        Row {
            anchors.horizontalCenter: parent.horizontalCenter
            IconButton {
                id: sleepButton
                icon: QIcon("system-suspend")
                iconWidth: 22
                iconHeight: 22
                text: "Sleep"
                onClicked: sleepClicked()
            }

            IconButton {
                id: hibernateButton
                icon: QIcon("system-suspend-hibernate")
                iconWidth: 22
                iconHeight: 22
                text: "Hibernate"
                onClicked: hibernateClicked()
            }
        }
    }
}
