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
import org.kde.plasma.components 0.1 as PlasmaComponents

PlasmaCore.FrameSvgItem {
    id: shutdownUi
    property int realMarginTop: margins.top
    property int realMarginBottom: margins.bottom
    property int realMarginLeft: margins.left
    property int realMarginRight: margins.right

    width: realMarginLeft + 2 * buttonsLayout.width + realMarginRight
    height: realMarginTop + automaticallyDoLabel.height + buttonsLayout.height + realMarginBottom

    imagePath: "dialogs/shutdowndialog"

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

        anchors {
            top: parent.top
            topMargin: realMarginTop
            bottom: parent.bottom
            bottomMargin: realMarginBottom
            left: parent.left
            leftMargin: realMarginLeft
            right: parent.right
            rightMargin: realMarginRight
        }

        svg: PlasmaCore.Svg {
            imagePath: "dialogs/shutdowndialog"
        }
        elementId: "center"
    }

    Component.onCompleted: {
        // Hacky but works :-)
        logoutButton.width = buttonsLayout.width
        shutdownButton.width = buttonsLayout.width
        rebootButton.width = buttonsLayout.width

        if (margins.left == 0) {
            realMarginTop = 9
            realMarginBottom = 7
            realMarginLeft = 12
            realMarginRight = 12
        }

        if (leftPicture.naturalSize.width < 1) {
            background.elementId = "background"
            shutdownUi.width += realMarginLeft + realMarginRight
            shutdownUi.height += realMarginTop + realMarginBottom
            automaticallyDoLabel.anchors.topMargin = 2*realMarginTop
            automaticallyDoLabel.anchors.rightMargin = 2*realMarginRight
            leftPicture.anchors.leftMargin = 2*realMarginLeft
            buttonsLayout.anchors.rightMargin = 2*realMarginRight
        }

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
        }

        focusedButton.focusedButton = true

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
                    focusedButton.clicked()
                // following code is required to provide a clean way to translate strings
                } else if (focusedButton.text == logoutButton.text) {
                    automaticallyDoLabel.text = i18np("Logging out in 1 second.",
                                                      "Logging out in %1 seconds.", automaticallyDoSeconds)
                } else if (focusedButton.text == shutdownButton.text) {
                    automaticallyDoLabel.text = i18np("Turning off computer in 1 second.",
                                                      "Turning off computer in %1 seconds.", automaticallyDoSeconds)
                } else if (focusedButton.text == rebootButton.text) {
                    automaticallyDoLabel.text = i18np("Restarting computer in 1 second.",
                                                      "Restarting computer in %1 seconds.", automaticallyDoSeconds)
                } else {
                    automaticallyDoLabel.text = ""
                }

                --automaticallyDoSeconds;
            }
        }
    }

    Text {
        id: automaticallyDoLabel
        text: " "
        // pixelSize does not work with PlasmaComponents.Label, so I am using a Text element here.
        font.pixelSize: 11
        color: theme.textColor
        anchors {
            top: parent.top
            topMargin: realMarginTop
            right: parent.right
            rightMargin: realMarginRight
        }
    }

    PlasmaCore.SvgItem {
        id: leftPicture
        width: buttonsLayout.width
        height: width * naturalSize.height / naturalSize.width
        smooth: true
        anchors {
            verticalCenter: parent.verticalCenter
            left: parent.left
            leftMargin: realMarginLeft
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
            topMargin: 4
            right: parent.right
            rightMargin: realMarginRight
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

                onPressed: {
                    if (shutdownUi.focusedButton != logoutButton) {
                        shutdownUi.focusedButton.focusedButton = false
                        shutdownUi.focusedButton = logoutButton
                        focusedButton = true
                        focus = true
                    }
                }
            }

            KSMButton {
                id: shutdownButton
                text: i18n("Turn Off Computer")
                iconSource: "system-shutdown"
                height: 32
                anchors.right: parent.right
                visible: (choose || sdtype == ShutdownType.ShutdownTypeHalt)
                menu: true

                onClicked: {
                    console.log("haltRequested");
                    haltRequested()
                }

                onPressAndHold: {
                    console.log("shutdownMenuRequested");
                    shutdownButtonColumn.visible = !shutdownButtonColumn.visible
                    adjustSizeTimer.running = true
                }

                onPressed: {
                    if (shutdownUi.focusedButton != shutdownButton) {
                        shutdownUi.focusedButton.focusedButton = false
                        shutdownUi.focusedButton = shutdownButton
                        focusedButton = true
                        focus = true
                    }
                }
            }

            // Some themes do not adjust dialog size automatically, so update it manually.
            Timer {
                id: adjustSizeTimer
                repeat: false
                running: false
                interval: 100

                onTriggered: {
                    shutdownUi.height = realMarginTop + automaticallyDoLabel.height + buttonsLayout.height + realMarginBottom
                    if (margins.left == 0) {
                        shutdownUi.height += realMarginTop + realMarginBottom
                    }
                }
            }

            PlasmaComponents.ButtonColumn {
                id: shutdownButtonColumn
                visible: false
                anchors.right: parent.right

                Component.onCompleted: {
                    standbyButton.width = shutdownButtonColumn.width
                    sleepButton.width = shutdownButtonColumn.width
                    hibernateButton.width = shutdownButtonColumn.width
                }

                PlasmaComponents.Button {
                    id: standbyButton
                    text: i18n("Standby")
                    visible: shutdownButton.visible && spdMethods.StandbyState
                    onClicked: {
                        console.log("suspendRequested(Solid::PowerManagement::StandbyState)")
                        suspendRequested(1); // Solid::PowerManagement::StandbyState
                    }
                }
                PlasmaComponents.Button {
                    id: sleepButton
                    text: i18n("Suspend to RAM")
                    visible: shutdownButton.visible && spdMethods.SuspendState
                    onClicked: {
                        console.log("suspendRequested(Solid::PowerManagement::SuspendState)")
                        suspendRequested(2); // Solid::PowerManagement::SuspendState
                    }
                }
                PlasmaComponents.Button {
                    id: hibernateButton
                    text: i18n("Suspend to Disk")
                    visible: shutdownButton.visible && spdMethods.HibernateState
                    onClicked: {
                        console.log("suspendRequested(Solid::PowerManagement::HibernateState)")
                        suspendRequested(3); // Solid::PowerManagement::HibernateState
                    }
                }
            }

            KSMButton {
                id: rebootButton
                text: i18n("Restart Computer")
                iconSource: "system-reboot"
                height: 32
                anchors.right: parent.right
                menu: true

                onClicked: {
                    console.log("rebootRequested");
                    rebootRequested()
                }

                onPressAndHold: {
                    console.log("rebootMenuRequested");
                    rebootButtonColumn.visible = !rebootButtonColumn.visible
                    adjustSizeTimer.running = true
                }

                onPressed: {
                    if (shutdownUi.focusedButton != rebootButton) {
                        shutdownUi.focusedButton.focusedButton = false
                        shutdownUi.focusedButton = rebootButton
                        focusedButton = true
                        focus = true
                    }
                }
            }

            PlasmaComponents.ButtonColumn {
                id: rebootButtonColumn
                visible: false
                anchors.right: parent.right

                Component.onCompleted: {
                    rebootOption0.width = rebootButtonColumn.width
                    rebootOption1.width = rebootButtonColumn.width
                    rebootOption2.width = rebootButtonColumn.width
                    rebootOption3.width = rebootButtonColumn.width
                    rebootOption4.width = rebootButtonColumn.width
                }

                PlasmaComponents.Button {
                    id: rebootOption0
                    text: rebootOptions["options"][0]
                    visible: rebootOptions["options"][0] != undefined && rebootOptions["options"][0] != ""
                    onClicked: {
                        console.log("rebootRequested: " + text)
                        rebootRequested2(0);
                    }
                }
                PlasmaComponents.Button {
                    id: rebootOption1
                    text: rebootOptions["options"][1]
                    visible: rebootOptions["options"][1] != undefined && rebootOptions["options"][1] != ""
                    onClicked: {
                        console.log("rebootRequested: " + text)
                        rebootRequested2(1);
                    }
                }
                PlasmaComponents.Button {
                    id: rebootOption2
                    text: rebootOptions["options"][2]
                    visible: rebootOptions["options"][2] != undefined && rebootOptions["options"][2] != ""
                    onClicked: {
                        console.log("rebootRequested: " + text)
                        rebootRequested2(2);
                    }
                }
                PlasmaComponents.Button {
                    id: rebootOption3
                    text: rebootOptions["options"][3]
                    visible: rebootOptions["options"][3] != undefined && rebootOptions["options"][3] != ""
                    onClicked: {
                        console.log("rebootRequested: " + text)
                        rebootRequested2(3);
                    }
                }
                PlasmaComponents.Button {
                    id: rebootOption4
                    text: rebootOptions["options"][4]
                    visible: rebootOptions["options"][4] != undefined && rebootOptions["options"][4] != ""
                    onClicked: {
                        console.log("rebootRequested: '" + text + "'")
                        rebootRequested2(4);
                    }
                }
                PlasmaComponents.Button {
                    id: rebootOption5
                    text: rebootOptions["options"][5]
                    visible: rebootOptions["options"][5] != undefined && rebootOptions["options"][5] != ""
                    onClicked: {
                        console.log("rebootRequested: '" + text + "'")
                        rebootRequested2(5);
                    }
                }
            }
        }

        KSMButton {
            id: cancelButton
            text: i18n("Cancel")
            iconSource: "dialog-cancel"
            smallButton: true
            height: 22
            anchors.right: parent.right

            onClicked: {
                cancelRequested()
            }
        }
    }
}
