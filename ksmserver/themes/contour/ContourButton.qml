/*
 *   Copyright 2011 Lamarque V. Souza <lamarque@kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU Library General Public License as
 *   published by the Free Software Foundation; either version 2 or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU Library General Public License for more details
 *
 *   You should have received a copy of the GNU Library General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.qtextracomponents 0.1

PlasmaCore.FrameSvgItem {
    id: button
    width: mainColumn.width
    height: mainColumn.height

    property string iconSource
    property alias iconSize: iconElement.width
    property alias text: labelElement.text
    property alias font: labelElement.font

    signal clicked()

    PlasmaCore.Theme {
        id: theme
    }

    Column {
        id: mainColumn

        QIconItem {
            id: iconElement
            icon: QIcon(iconSource)
            height: width
    
            MouseArea {
                anchors.fill: parent
                onClicked: button.clicked()
                onPressed: button.state = "Pressed"
                onReleased: button.state = "Normal"
            }
        }
        Text {
            id: labelElement
            anchors.horizontalCenter: iconElement.horizontalCenter
            horizontalAlignment: Text.AlignHCenter
            color: theme.textColor
            // Use theme.defaultFont in plasma-mobile and
            // theme.font in plasma-desktop.
            font.family: theme.defaultFont.family
            font.bold: theme.defaultFont.bold
            font.capitalization: theme.defaultFont.capitalization
            font.italic: theme.defaultFont.italic
            font.weight: theme.defaultFont.weight
            font.underline: theme.defaultFont.underline
            font.wordSpacing: theme.defaultFont.wordSpacing
        }
    }

    states: [
        State {
            name: "Normal"
            PropertyChanges { target: mainColumn; scale: 1.0}
        },
        State {
            name: "Pressed"
            PropertyChanges { target: mainColumn; scale: 0.9}
        }
    ]

    transitions: [
        Transition {
            NumberAnimation { properties: "scale"; duration: 50 }
        }
    ]
}
