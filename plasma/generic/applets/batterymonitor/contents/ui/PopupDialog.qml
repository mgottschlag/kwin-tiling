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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as Components

Item {
    id: dialog
    width: childrenRect.width+24
    height: childrenRect.height+24

    property int percent
    property bool hasBattery
    property bool pluggedIn
    property alias screenBrightness: brightnessSlider.value
    property alias showSuspendButton: suspendButton.visible
    property alias showHibernateButton: hibernateButton.visible

    property int ram: 0
    property int disk: 1

    signal suspendClicked(int type)
    signal brightnessChanged(int screenBrightness)
    signal powermanagementChanged(bool checked)

    Column {
        id: labels
        spacing: 8
        anchors {
            top: parent.top
            left: parent.left
            margins: 12
        }

        Components.Label {
            text: i18n("Battery:")
            anchors.right: parent.right
        }

        Components.Label {
            text: i18n("AC Adapter:")
            anchors.right: parent.right
        }

        Components.Label {
            text: i18nc("Label for power management inhibition", "Power management enabled:")
            anchors.right: parent.right
        }

        Components.Label {
            text: i18n("Screen Brightness:")
            anchors.right: parent.right
        }
    }

    Column {
        id: values
        spacing: 8
        anchors {
            top: parent.top
            left: labels.right
            margins: 12
        }

        Components.Label {
            text: dialog.hasBattery ? stringForState(pluggedIn, percent) : i18nc("Battery is not plugged in", "Not present")
            font.weight: Font.Bold
        }

        Components.Label {
            text: dialog.pluggedIn ? i18n("Plugged in") : i18n("Not plugged in")
            font.weight: Font.Bold
        }

        Components.CheckBox {
            checked: true
            onClicked: powermanagementChanged(checked)
        }

        Components.Slider {
            id: brightnessSlider
            minimumValue: 0
            maximumValue: 100
            stepSize: 10
            onValueChanged: brightnessChanged(value)
        }
    }

    Row {
        anchors {
            top: values.bottom
            topMargin: 10
            right: values.right
        }

        IconButton {
            id: suspendButton
            icon: QIcon("system-suspend")
            iconWidth: 22
            iconHeight: 22
            text: i18nc("Suspend the computer to RAM; translation should be short", "Sleep")
            onClicked: suspendClicked(ram)
        }

        IconButton {
            id: hibernateButton
            icon: QIcon("system-suspend-hibernate")
            iconWidth: 22
            iconHeight: 22
            text: i18nc("Suspend the computer to disk; translation should be short", "Hibernate")
            onClicked: suspendClicked(disk)
        }
    }

    BatteryIcon {
        monochrome: false
        hasBattery: dialog.hasBattery
        percent: dialog.percent
        pluggedIn: dialog.pluggedIn
        anchors {
            top: parent.top
            right: values.right
            topMargin: 12
        }
        width: 50
        height: 50
    }
}
