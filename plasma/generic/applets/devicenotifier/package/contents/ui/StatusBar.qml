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

Item {
    id: statusBar
    height: statusText.height

    function show(error, details, udi) {
        statusText.text = error;
        hideStatusTextTimer.restart();
    }

    Timer {
        id: hideStatusTextTimer
        interval: 7500
        onTriggered: statusText.text="";
    }

    // FIXME: This SVG does not show up!
    PlasmaCore.FrameSvgItem {
        id: closeBtn
        width: 16
        imagePath: "widgets/configuration-icons"
        prefix: "close"
        anchors {
            top: parent.top
            right: parent.right
        }
    }

    Text {
        id: statusText
        anchors {
            left: parent.left
            right: closeBtn.left
            bottom: parent.bottom
        }
        clip: true
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignBottom
        // FIXME: determine the height according to the contents
        // instead of just 28
        height: text=="" ? 0 : 28

        Behavior on height { NumberAnimation { duration: 200 } }
    }
}
