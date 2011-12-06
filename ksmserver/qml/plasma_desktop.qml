// -*- coding: iso-8859-1 -*-
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

import QtQuick 1.1
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.components 0.1
import org.kde.qtextracomponents 0.1

PlasmaCore.FrameSvgItem {
    id: shutdownUi
    width: margins.left + 2 * buttonsLayout.width + margins.right
    height: margins.top + automaticallyDoLabel.height + buttonsLayout.height + margins.bottom
    imagePath: "dialogs/shutdowndialog"
    enabledBorders: PlasmaCore.FrameSvg.AllBorders

    signal logoutRequested()
    signal haltRequested()
    signal suspendRequested(int spdMethod)
    signal rebootRequested()
    signal rebootRequested2(int opt)
    signal cancelRequested()
    signal lockScreenRequested()

    property variant focusedButton: 0
    property variant lastButton: 0
    property int automaticallyDoSeconds: 30

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.SvgItem {
        id: background
        anchors.fill: parent

        svg: PlasmaCore.Svg {
            imagePath: "dialogs/shutdowndialog"
        }
        elementId: "center"
    }

    Component.onCompleted: {
        //console.log("margins: " + margins.left + ", " + margins.top + ", " + margins.right + ", " + margins.bottom);

        // Hacky but works :-)
        logoutButton.width = buttonsLayout.width
        shutdownButton.width = buttonsLayout.width
        rebootButton.width = buttonsLayout.width

        if (background.naturalSize.width < 1) {
            background.elementId = "background"
        }

        if (leftPicture.naturalSize.width < 1) {
            shutdownUi.width -= buttonsLayout.width
            automaticallyDoLabel.width -= buttonsLayout.width
        }

        //console.log("maysd("+maysd+") choose("+choose+") sdtype("+sdtype+")");
        if (choose || sdtype == ShutdownType.ShutdownTypeNone) {
            if (sdtype == ShutdownType.ShutdownTypeNone) {
                focusedButton = logoutButton
            }
        }

        if (maysd) {
            if(choose || sdtype == ShutdownType.ShutdownTypeHalt) {
                if (sdtype == ShutdownType.ShutdownTypeHalt) {
                    focusedButton = shutdownButton
                }
            }

            if (choose || sdtype == ShutdownType.ShutdownTypeReboot) {
                if (sdtype == ShutdownType.ShutdownTypeReboot) {
                    focusedButton = rebootButton
                }

                // TODO: add reboot menu
            }
        } else {
            shutdownButton.opacity = 0
            standbyButton.opacity = 0
            suspendToRamButton.opacity = 0
            suspendToDiskButton.opacity = 0
        }

        timer.interval = 1000;
        timer.running = true;
    }

    Timer {
        id: timer
        repeat: true
        running: false

        onTriggered: {
            if (focusedButton != lastButton) {
                lastButton = focusedButton
                automaticallyDoSeconds = 30
            }
            if (focusedButton != 0) {
                if (automaticallyDoSeconds <= 0) { // timeout is at 0, do selected action
                    //console.log("focusedButton == " + focusedButton.text);
                    focusedButton.clicked()
                // following code is required to provide a clean way to translate strings
                } else if (focusedButton.text == logoutButton.text) {
                    //console.log("focusedButton == " + focusedButton.text);
                    automaticallyDoLabel.text = i18np("Logging out in 1 second.",
                                                      "Logging out in %1 seconds.", automaticallyDoSeconds)
                } else if (focusedButton.text == shutdownButton.text) {
                    //console.log("focusedButton == " + focusedButton.text);
                    automaticallyDoLabel.text = i18np("Turning off computer in 1 second.",
                                                      "Turning off computer in %1 seconds.", automaticallyDoSeconds)
                } else if (focusedButton.text == rebootButton.text) {
                    //console.log("ocusedButton == " + focusedButton.text);
                    automaticallyDoLabel.text = i18np("Restarting computer in 1 second.",
                                                      "Restarting computer in %1 seconds.", automaticallyDoSeconds)
                } else {
                    automaticallyDoLabel.text = ""
                }

                --automaticallyDoSeconds;
            }
        }
    }

    Label {
        id: automaticallyDoLabel
        text: " "
        font.pixelSize: 11
        anchors {
            top: parent.top
            topMargin: parent.margins.top
            rightMargin: parent.margins.right
            right: parent.right
        }
    }

    PlasmaCore.SvgItem {
        id: leftPicture
        width: buttonsLayout.width
        height: width * naturalSize.height / naturalSize.width
        smooth: true
        anchors {
            top: automaticallyDoLabel.top
            left: parent.left
            leftMargin: parent.margins.left
        }

        svg: PlasmaCore.Svg {
            imagePath: "dialogs/shutdowndialog"
        }
        elementId: "picture"
    }

    Column {
        id: buttonsLayout
        spacing: 9
        anchors {
            top: automaticallyDoLabel.bottom
            right: parent.right
            rightMargin: parent.margins.right
        }

        Column {
            spacing: 4

            KSMButton {
                id: logoutButton
                text: i18n("Logout")
                iconSource: "system-log-out"
                height: 32
                anchors.right: parent.right
                visible: (choose || sdtype == ShutdownType.ShutdownTypeNone)

                onClicked: {
                    console.log("logoutRequested");
                    logoutRequested()
                }
            }

            KSMButton {
                id: shutdownButton
                text: i18n("Turn Off Computer")
                iconSource: "system-shutdown"
                height:32
                anchors.right: parent.right
                property ContextMenu contextMenu
                visible: (choose || sdtype == ShutdownType.ShutdownTypeHalt)

                onClicked: {
                    console.log("shutdownMenuRequested");
                    if (!contextMenu) {
                        contextMenu = shutdownOptionsComponent.createObject(shutdownButton)
                    }
                    contextMenu.open()
                }

                /*Component.onCompleted: {
                    console.log("shutdownButton.onCompleted");
                    if (contextMenu.valid && !contextMenu) {
                        contextMenu = shutdownOptionsComponent.createObject(shutdownButton)
                    }
                }*/
            }

            Component {
                id: shutdownOptionsComponent
                ContextMenu {
                    visualParent: shutdownButton
                    MenuItem {
                        id: shutdown
                        text: i18n("Turn Off Computer")
                        //height: shutdownButton.visible ? height : 0
                        visible: shutdownButton.visible
                        onClicked: {
                            console.log("haltRequested")
                            haltRequested()
                        }
                    }
                    MenuItem {
                        id: standby
                        text: i18n("Standby")
                        //height: shutdownButton.visible && spdMethods.StandbyState ? height : 0
                        visible: shutdownButton.visible && spdMethods.StandbyState
                        onClicked: {
                            console.log("suspendRequested(Solid::PowerManagement::StandbyState)")
                            suspendRequested(1); // Solid::PowerManagement::StandbyState
                        }
                    }
                    MenuItem {
                        id: sleep
                        text: i18n("Suspend to RAM")
                        //height: shutdownButton.visible && spdMethods.SuspendState ? height : 0
                        visible: shutdownButton.visible && spdMethods.SuspendState
                        onClicked: {
                            console.log("suspendRequested(Solid::PowerManagement::SuspendState)")
                            suspendRequested(2); // Solid::PowerManagement::SuspendState
                        }
                    }
                    MenuItem {
                        id: hibernate
                        text: i18n("Suspend to Disk")
                        //height: shutdownButton.visible && spdMethods.HibernateState ? height : 0
                        visible: shutdownButton.visible && spdMethods.HibernateState
                        onClicked: {
                            console.log("suspendRequested(Solid::PowerManagement::HibernateState)")
                            suspendRequested(3); // Solid::PowerManagement::HibernateState
                        }
                    }
                }
            }

            KSMButton {
                id: rebootButton
                text: i18n("Restart Computer")
                iconSource: "system-reboot"
                height:32
                anchors.right: parent.right

                onClicked: {
                    console.log("rebootRequested");
                    rebootRequested()
                }
            }
        }

        KSMButton {
            id: cancelButton
            text: i18n("Cancel")
            iconSource: "dialog-cancel"
            height: 22
            anchors.right: parent.right

            onClicked: {
                cancelRequested()
            }
        }
    }
}
