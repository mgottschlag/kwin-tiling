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
import org.kde.qtextracomponents 0.1

Item {
    property alias icon: iconItem.icon
    property alias iconWidth: iconItem.width
    property alias iconHeight: iconItem.height
    property alias text: buttonText.text
    property int orientation

    signal clicked()

    QIconItem {
        id: iconItem
        scale: mouseArea.pressed ? 0.9 : 1
        smooth: true
        Component.onCompleted: {
            if (orientation==Qt.Horizontal) {
                anchors.verticalCenter = parent.verticalCenter;
                anchors.left = parent.left;
                anchors.leftMargin = 5;
            }
            else if (orientation==Qt.Vertical) {
                anchors.horizontalCenter = parent.horizontalCenter;
                anchors.top = parent.top;
                anchors.topMargin = 5;
            }
        }
    }

    Text {
        id: buttonText
        Component.onCompleted: {
            if (orientation==Qt.Horizontal) {
                anchors.verticalCenter = parent.verticalCenter;
                anchors.left = iconItem.right;
                anchors.leftMargin = 5;
            }
            else if (orientation==Qt.Vertical) {
                anchors.horizontalCenter = parent.horizontalCenter;
                anchors.top = iconItem.bottom;
                anchors.topMargin = 5;
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: highlightItem
        anchors.fill: parent
        scale: mouseArea.pressed ? 0.95 : 1
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: mouseArea.containsMouse ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
    }
}
