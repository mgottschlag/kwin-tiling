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
import org.kde.plasma.components 0.1 as PlasmaComponents

Item {
    property alias switchUserSupported: sessions.switchUserSupported
    signal activateSession()
    signal startNewSession()
    signal cancel()
    Sessions {
        id: sessions
    }
    ListView {
        model: sessions.model
        id: userSessionsView
        anchors {
            left: parent.left
            right: parent.right
            bottom: buttonRow.top
            bottomMargin: 5
        }
        height: parent.height - explainText.implicitHeight - buttonRow.height - 10

        delegate: PlasmaComponents.ListItem {
            content: PlasmaComponents.Label {
                text: session + "(" + location + ")"
            }
        }
        highlight: PlasmaComponents.Highlight {
            hover: true
            width: parent.width
        }
        focus: true
        MouseArea {
            anchors.fill: parent
            onClicked: userSessionsView.currentIndex = userSessionsView.indexAt(mouse.x, mouse.y)
            onDoubleClicked: {
                sessions.activateSession(userSessionsView.indexAt(mouse.x, mouse.y));
                activateSession();
            }
        }
    }
    PlasmaComponents.Label {
        id: explainText
        text: i18n("The current session will be hidden " +
                    "and a new login screen or an existing session will be displayed.\n" +
                    "An F-key is assigned to each session; " +
                    "F%1 is usually assigned to the first session, " +
                    "F%2 to the second session and so on. " +
                    "You can switch between sessions by pressing " +
                    "Ctrl, Alt and the appropriate F-key at the same time. " +
                    "Additionally, the KDE Panel and Desktop menus have " +
                    "actions for switching between sessions.",
                    7, 8)
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
    }
    PlasmaComponents.ButtonRow {
        id: buttonRow
        exclusive: false

        PlasmaComponents.Button {
            id: activateSession
            text: i18n("Activate")
            iconSource: "fork"
            onClicked: {
                sessions.activateSession(userSessionsView.currentIndex);
                activateSession();
            }
        }
        PlasmaComponents.Button {
            id: newSession
            text: i18n("Start New Session")
            iconSource: "fork"
            visible: sessions.startNewSessionSupported
            onClicked: {
                sessions.startNewSession();
                startNewSession();
            }
        }
        PlasmaComponents.Button {
            id: cancelSession
            text: i18n("Cancel")
            iconSource: "dialog-cancel"
            onClicked: cancel()
        }
        anchors.bottom: parent.bottom
        anchors.horizontalCenter: userSessionsUI.horizontalCenter
    }
}
