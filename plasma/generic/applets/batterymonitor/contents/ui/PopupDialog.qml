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
    height: 200

    property int percent
    property bool pluggedIn
    property alias screenBrightness: brightnessSlider.value
    property int remainingMsec

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
            width: longestText.width
        }
        Components.Label {
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
                return i18n(txt);
            }
            font.weight: Font.Bold
        }
        
        Components.Label {
            text: i18n("AC Adapter:")
            horizontalAlignment: Text.AlignRight
            width: longestText.width
        }
        Components.Label {
            text: dialog.pluggedIn ? i18n("Plugged in") : i18n("Not plugged in")
            font.weight: Font.Bold
        }
        
        Components.Label {
            text: i18n("Time Remaining:")
            horizontalAlignment: Text.AlignRight
            width: longestText.width
        }
        Components.Label {
            text: {
                var msec = Number(remainingMsec);
                var hrs = Math.floor(msec/3600000);
                var mins = Math.floor((msec-(hrs*3600000))/60000);
                var txt = "";
                if (hrs==1) txt += "1 hour";
                else if (hrs>1) txt += hrs+" hours";
                
                if (mins>0 && hrs>0) txt += " and ";
                if (mins==1) txt += "1 minute";
                else if (mins>0) txt += mins+" minutes";
                
                return i18n(txt);
            }
            font.weight: Font.Bold
        }
        
        Components.Label {
            id: longestText
            text: i18n("Power management enabled:")
        }
        Components.Switch {
            checked: true
            onCheckedChanged: powermanagementChanged(checked)
        }
        
        Components.Label {
            text: i18n("Screen Brightness:")
            horizontalAlignment: Text.AlignRight
            width: longestText.width
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
