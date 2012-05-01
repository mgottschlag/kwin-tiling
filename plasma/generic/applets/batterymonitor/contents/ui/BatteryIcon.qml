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
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    
    property bool monochrome
    property bool hasBattery
    property int percent
    property bool pluggedIn
    
    PlasmaCore.Svg {
        id: svg
        imagePath: monochrome ? "icons/battery" : "widgets/battery-oxygen"
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent
        svg: svg
        elementId: "Battery"
    }

    PlasmaCore.SvgItem {
        anchors.fill: parent
        svg: svg
        elementId: hasBattery ? fillElement(percent) : "Unavailable"
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
        svg: svg
        elementId: "AcAdapter"
        visible: pluggedIn
    }
}
