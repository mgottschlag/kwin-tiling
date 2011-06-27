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
    id: dialog
    width: 387
    height: 180

    property int percent
    property bool pluggedIn
    property alias screenBrightness: brightnessSlider.value
    property alias currentProfileIndex: profiles.currentIndex

    signal sleepClicked
    signal hibernateClicked
    signal brightnessChanged(int screenBrightness)
    signal profileChanged(string profile)

    function addProfile(profile) {
        profiles.addItem(profile);
    }

    Item {
        id: labels
        width: parent.width/3
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
            text: "Power Profile:"
            anchors {
                top: adapterLabel.bottom
                topMargin: 10
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
            text: percent+"%" + " (" + (percent<100 ? (pluggedIn ? "charging" : "discharging") : "charged") + ")"
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

        PlasmaWidgets.ComboBox {
            id: profiles
            anchors {
                left: parent.left
                top: adapterValue.bottom
                topMargin: 5
                right: parent.right
            }
            onTextChanged: profileChanged(text)
        }

        PlasmaWidgets.Slider {
            id: brightnessSlider
            orientation: Qt.Horizontal
            minimum: 0
            maximum: 100
            /* TODO: implement this property */
            //tickInterval: 10
            anchors {
                left: parent.left
                top: adapterValue.bottom
                //topMargin: -5
                right: parent.right
            }
            onValueChanged: brightnessChanged(value)
        }

        IconButton {
            id: sleepButton
            icon: QIcon("system-suspend")
            text: "Sleep"
            orientation: Qt.Horizontal
            iconWidth: 32
            iconHeight: 32
            width: parent.width/2
            anchors {
                left: parent.left
                top: brightnessSlider.bottom
                bottom: parent.bottom
            }
            onClicked: sleepClicked()
        }

        IconButton {
            id: hibernateButton
            icon: QIcon("system-suspend-hibernate")
            text: "Hibernate"
            orientation: Qt.Horizontal
            iconWidth: 32
            iconHeight: 32
            width: parent.width/2
            anchors {
                right: parent.right
                top: brightnessSlider.bottom
                bottom: parent.bottom
            }
            onClicked: hibernateClicked()
        }
    }
}
