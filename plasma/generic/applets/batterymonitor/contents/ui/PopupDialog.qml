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
    property string batteryState
    property bool hasBattery
    property bool pluggedIn
    property alias screenBrightness: brightnessSlider.value
    property int remainingMsec
    property alias showSuspendButton: suspendButton.visible
    property alias showHibernateButton: hibernateButton.visible

    property int ram: 0
    property int disk: 1

    signal suspendClicked(int type)
    signal brightnessChanged(int screenBrightness)
    signal powermanagementChanged(bool checked)

    Column {
        id: labels
        spacing: 6
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
            anchors.bottomMargin: 12
        }

        Components.Label {
            text: i18nc("Label for remaining time", "Time Remaining:")
            visible: remainingTime.visible
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
        spacing: 6
        anchors {
            top: parent.top
            left: labels.right
            margins: 12
        }

        Components.Label {
            text: dialog.hasBattery ? stringForState(batteryState, percent) : i18nc("Battery is not plugged in", "Not present")
            font.weight: Font.Bold
        }

        Components.Label {
            text: dialog.pluggedIn ? i18n("Plugged in") : i18n("Not plugged in")
            font.weight: Font.Bold
            anchors.bottomMargin: 12
        }

        Components.Label {
            id: remainingTime
            text: formatDuration(remainingMsec);
            font.weight: Font.Bold
            visible: text!=""
        }

        Components.CheckBox {
            checked: true
            onClicked: powermanagementChanged(checked)
            x: 1
        }

        Components.Slider {
            id: brightnessSlider
            minimumValue: 0
            maximumValue: 100
            stepSize: 10
            onValueChanged: brightnessChanged(value)
        }
    }

    // TODO: give translated and formatted string with KGlobal::locale()->prettyFormatDuration(msec);
    function formatDuration(msec) {
        if (msec==0)
            return "";

        var time = new Date(msec);
        var hrs = i18np("1 hour", "%1 hours", time.getUTCHours());
        var mins = i18np("1 minute", "%1 minutes", time.getUTCMinutes());
        return hrs+", "+mins;
    }

    Row {
        anchors {
            top: values.bottom
            topMargin: 10
            right: values.right
        }

        Components.ToolButton {
            id: suspendButton
            iconSource: "system-suspend"
            text: i18nc("Suspend the computer to RAM; translation should be short", "Sleep")
            onClicked: suspendClicked(ram)
        }

        Components.ToolButton {
            id: hibernateButton
            iconSource: "system-suspend-hibernate"
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
