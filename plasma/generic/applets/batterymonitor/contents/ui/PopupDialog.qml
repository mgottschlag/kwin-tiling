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
    width: 400
    height: childrenRect.height

    property int percent
    property bool pluggedIn
    property alias screenBrightness: brightnessSlider.value
    property int remainingMsec
    property bool showRemainingTime

    signal sleepClicked
    signal hibernateClicked
    signal brightnessChanged(int screenBrightness)
    signal powermanagementChanged(bool checked)

    Grid {
        id: positioner
        columns: 2
        spacing: 5
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        Components.Label {
            text: i18n("Battery:")
            horizontalAlignment: Text.AlignRight
        }
        Components.Label {
            text: {
                if (percent == 0) {
                    return i18nc("Battery is not plugged in", "Not present");
                }
                var state;
                if (pluggedIn) {
                    if (percent<100) return i18n("%1% (charging)", percent);
                    else return i18n("%1% (charged)", percent);
                } else {
                    return i18n("%1% (discharging)", percent);
                }
            }
            font.weight: Font.Bold
        }

        Components.Label {
            text: i18n("AC Adapter:")
            horizontalAlignment: Text.AlignRight
        }
        Components.Label {
            text: dialog.pluggedIn ? i18n("Plugged in") : i18n("Not plugged in")
            font.weight: Font.Bold
        }

        Components.Label {
            text: i18nc("Label for remaining time", "Time Remaining:")
            horizontalAlignment: Text.AlignRight
            visible: timeRemain.visible
        }
        Components.Label {
            id: timeRemain
            // TODO: give translated and formatted string with KGlobal::locale()->prettyFormatDuration(msec);
            text: {
                var time = new Date(remainingMsec);
                var hrs = i18np("1 hour", "%1 hours", time.getUTCHours());
                var mins = i18np("1 minute", "%1 minutes", time.getUTCMinutes());
                return hrs+", "+mins;
            }
            font.weight: Font.Bold
            visible: text!="" && dialog.showRemainingTime
        }

        Components.Label {
            text: i18nc("Label for powermanagement inhibition", "Power management enabled:")
        }
        Components.Switch {
            checked: true
            onCheckedChanged: powermanagementChanged(checked)
        }

        Components.Label {
            text: i18n("Screen Brightness:")
            horizontalAlignment: Text.AlignRight
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
            top: positioner.bottom
            topMargin: 10
            right: parent.right
        }

        IconButton {
            id: sleepButton
            icon: QIcon("system-suspend")
            iconWidth: 22
            iconHeight: 22
            text: i18nc("Suspend the computer to RAM; translation should be short", "Sleep")
            onClicked: sleepClicked()
        }

        IconButton {
            id: hibernateButton
            icon: QIcon("system-suspend-hibernate")
            iconWidth: 22
            iconHeight: 22
            text: i18nc("Suspend the computer to disk; translation should be short", "Hibernate")
            onClicked: hibernateClicked()
        }
    }
}
