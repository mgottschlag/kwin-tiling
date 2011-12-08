/*
    Copyright (C) 2011  Lamarque Souza <lamarque@kde.org>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License along
    with this program; if not, write to the Free Software Foundation, Inc.,
    51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
*/

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

PlasmaCore.FrameSvgItem {
    id: button
    property string iconSource
    property alias text: labelElement.text
    property bool smallButton: false

    signal clicked()

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.SvgItem {
        id: background
        anchors.fill: parent

        svg: PlasmaCore.Svg {
            imagePath: "dialogs/shutdowndialog"
        }
        elementId: "button-normal"
    }

    PlasmaComponents.Label {
        id: labelElement
        color: theme.textColor
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: 5
        }

        onPaintedWidthChanged: {
            button.width = Math.max(button.width, 5 + labelElement.width + 10 + iconElement.width + 5)
        }        
    }

    QIconItem {
        id: iconElement
        icon: QIcon(iconSource)
        width: height
        height: parent.height - 6

        anchors {
            verticalCenter: parent.verticalCenter
            right: parent.right
            rightMargin: 3
        }
    }

    Component.onCompleted: {
        background.elementId = button.smallButton ? "button-small-normal" : "button-normal"
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onClicked: button.clicked()
        onPressAndHold: {
            /*if (contextMenu.valid && contextMenu) {
                console.log("Lamarque onPressAndHold: opening menu");
                contextMenu.open()
            }*/
        }
        onPressed: button.state = "Pressed"
        onReleased: button.state = "Normal"
        onEntered: {
            background.elementId = button.smallButton ? "button-small-hover" : "button-hover"
        }
        onExited: {
            background.elementId = button.smallButton ? "button-small-normal" : "button-normal"
        }
    }

    states: [
        State {
            name: "Normal"
            PropertyChanges { target: button; scale: 1.0}
        },
        State {
            name: "Pressed"
            PropertyChanges { target: button; scale: 0.9}
        }
    ]
    
    transitions: [
        Transition {
            NumberAnimation { properties: "scale"; duration: 50 }
        }
    ]
}
