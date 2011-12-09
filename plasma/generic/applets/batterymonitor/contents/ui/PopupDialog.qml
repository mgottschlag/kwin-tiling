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
    signal profileChanged(string profile)

    function addProfile(profile)    { profiles.addItem(profile);        }
    function setProfile (index)     { profiles.currentIndex = index;    }
    function clearProfiles()        { profiles.clear();                 }

    Item {
        id: labels
        width: 170
        height: parent.height
        anchors {
            left: parent.left
            top: parent.top
            topMargin: 10
            bottom: parent.bottom
        }

        Text {
            id: batteryLabel
            text: "Battery:"
            anchors {
                right: parent.right
                rightMargin: 10
            }
        }

        Text {
            id: adapterLabel
            text: "AC Adapter:"
            anchors {
                top: batteryLabel.bottom
                topMargin: 10
                right: parent.right 
                rightMargin: 10
            }
        }

        Text {
            id: profileLabel
            //text: "Power Profile:"
            text: "Power management\nenabled:"
            horizontalAlignment: Text.AlignRight
            anchors {
                top: adapterLabel.bottom
                topMargin: 10
                left: parent.left
                right: parent.right 
                rightMargin: 10
            }
        }

        Text {
            id: brightnessLabel
            text: "Screen Brightness:"
            anchors {
                top: profileLabel.bottom
                topMargin: 10
                right: parent.right 
                rightMargin: 10
            }
        }
    }

    Item {
        id: values
        anchors {
            left: labels.right
            right: parent.right
            top: parent.top
            topMargin: 10
            bottom: parent.bottom
        }

        Text {
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
            anchors {
                left: parent.left
            }
        }

        Text {
            id: adapterValue
            text: pluggedIn ? "Plugged in" : "Not plugged in"
            font.bold: true
            anchors {
                left: parent.left
                top: batteryValue.bottom
                topMargin: 10
            }
        }

        /*PlasmaWidgets.ComboBox {
            id: profiles
            anchors {
                left: parent.left
                top: adapterValue.bottom
                topMargin: 5
                right: parent.right
            }
            onTextChanged: profileChanged(text)
        }*/
        Components.CheckBox {
            id: profiles
            checked: true
            anchors {
                left: parent.left
                top: adapterValue.bottom
                topMargin: 15
            }
        }

        Components.Slider {
            id: brightnessSlider
            minimumValue: 0
            maximumValue: 100
            stepSize: 10
            anchors {
                left: parent.left
                top: profiles.bottom
                topMargin: 6
                right: parent.right
            }
            onValueChanged: brightnessChanged(value)
        }

        IconButton {
            id: sleepButton
            icon: QIcon("system-suspend")
            iconWidth: 22
            iconHeight: 22
            text: "Sleep"
            anchors {
                left: parent.left
                leftMargin: 15
                top: brightnessSlider.bottom
                topMargin: 10
            }
            onClicked: sleepClicked()
        }

        IconButton {
            id: hibernateButton
            icon: QIcon("system-suspend-hibernate")
            iconWidth: 22
            iconHeight: 22
            text: "Hibernate"
            anchors {
                left: sleepButton.right
                leftMargin: 15
                top: brightnessSlider.bottom
                topMargin: 10
            }
            onClicked: hibernateClicked()
        }
    }
}
