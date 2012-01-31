/********************************************************************
 KSld - the KDE Screenlocker Daemon
 This file is part of the KDE project.

Copyright (C) 2011 Martin Gräßlin <mgraesslin@kde.org>

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*********************************************************************/
import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1
import org.kde.kscreenlocker 1.0

Item {
    id: lockScreen
    state: "UNLOCK"
    signal unlockRequested()
    property alias capsLockOn: unlockUI.capsLockOn

    PlasmaCore.Theme {
        id: theme
    }

    Image {
        id: background
        anchors.fill: parent
        source: theme.wallpaperPathForSize(parent.width, parent.height)
    }

    PlasmaCore.FrameSvgItem {
        id: dialog
        anchors.centerIn: parent
        imagePath: "translucent/dialogs/background"
        width: unlockUI.implicitWidth * 1.5 + margins.left + margins.right
        height: unlockUI.implicitHeight * 1.5 + margins.top + margins.bottom
    }

    Greeter {
        id: unlockUI
        y: parent.height/2
        x: parent.width/2 - implicitWidth/2
        focus: true
        switchUserEnabled: userSessionsUI.switchUserSupported

        Connections {
            onAccepted: lockScreen.unlockRequested()
            onSwitchUserClicked: {
                // TODO: load if not loaded
                lockScreen.state = "SESSION"
            }
        }
    }

    // TODO: loader
    SessionSwitching {
        id: userSessionsUI
        anchors {
            fill: dialog
            leftMargin: dialog.margins.left
            rightMargin: dialog.margins.right
            topMargin: dialog.margins.top
            bottomMargin: dialog.margins.bottom
        }
        Connections {
            onCancel: lockScreen.state = "UNLOCK"
            onActivateSession: lockScreen.state = "UNLOCK"
            onStartNewSession: lockScreen.state = "UNLOCK"
        }
    }

    states: [
        State {
            name: "UNLOCK"
            PropertyChanges {target: unlockUI; visible: true}
            PropertyChanges {target: userSessionsUI; visible: false}
        },
        State {
            name: "SESSION"
            PropertyChanges {target: unlockUI; visible: false}
            PropertyChanges {target: userSessionsUI; visible: true}
        }
    ]
}
