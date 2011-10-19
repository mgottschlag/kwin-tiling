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
    visible: statusText.text!=""
    // FIXME: determine the height according to the contents
    // instead of just 28
    height: visible ? 28 : 0

    Behavior on height { NumberAnimation { duration: 200 } }

    function show(error, details, udi) {
        statusText.text = error;
        hideStatusTextTimer.restart();
    }

    function close() {
        statusText.text="";
    }

    Timer {
        id: hideStatusTextTimer
        interval: 7500
        onTriggered: close();
    }

    PlasmaCore.Svg {
        id: iconsSvg
        imagePath: "widgets/configuration-icons"
    }

    PlasmaCore.SvgItem {
        id: closeBtn
        property int size: 16
        width: size
        height: size
        svg: iconsSvg
        elementId: "close"
        anchors {
            top: parent.top
            right: parent.right
        }
    }

    MouseArea {
        id: closeBtnMouseArea
        anchors.fill: closeBtn
        onClicked: {
            hideStatusTextTimer.stop();
            close();
        }
    }

    PlasmaCore.SvgItem {
        id: detailsBtn
        width: closeBtn.size
        height: closeBtn.size
        svg: iconsSvg
        elementId: "restore"
        anchors {
            top: parent.top
            right: closeBtn.left
            rightMargin: 5
        }
    }

    Text {
        id: statusText
        anchors {
            top: parent.top
            left: parent.left
            right: detailsBtn.left
            bottom: parent.bottom
        }
        clip: true
        wrapMode: Text.WordWrap
        verticalAlignment: Text.AlignBottom
    }
}
