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

/**
 TODO:
 . use kde-runtime/plasma/declarativeimports/plasmacomponents/qml/ContextMenu.qml
 instead of a custom ContextMenu component.
 . find a way to make tab stop work without the extra TAB key press.
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

    /* This is not necessary for themes that follow the specification.
       Uncomment this and [1] below if the dialog appears without borders or background.
       You should not use broken themes though.
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
    }*/

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
            // [1]
            //background.elementId = "background"
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
            }
        }

        focusedButton.forceActiveFocus()
        timer.running = true;
    }

    Timer {
        id: timer
        repeat: true
        running: false
        interval: 1000

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
        font.pixelSize: theme.desktopFont.pointSize*0.9
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

    FocusScope {
        id: scope
        width: buttonsLayout.width
        height: buttonsLayout.height

        anchors {
            top: automaticallyDoLabel.bottom
            topMargin: 4
            right: parent.right
            rightMargin: realMarginRight
        }

        Column {
            id: buttonsLayout
            spacing: 9

            Column {
                spacing: 4

                KSMButton {
                    id: logoutButton
                    text: i18n("Logout")
                    iconSource: "system-log-out"
                    height: 32
                    anchors.right: parent.right
                    visible: (choose || sdtype == ShutdownType.ShutdownTypeNone)
                    tabStopNext: shutdownButton
                    tabStopBack: cancelButton

                    onClicked: {
                        console.log("main.qml: logoutRequested")
                        logoutRequested()
                    }

                    onActiveFocusChanged: {
                        shutdownUi.focusedButton = logoutButton
                    }
                }

                KSMButton {
                    id: shutdownButton
                    text: i18n("Turn Off Computer")
                    iconSource: "system-shutdown"
                    height: 32
                    anchors.right: parent.right
                    visible: (choose || sdtype == ShutdownType.ShutdownTypeHalt)
                    menu: spdMethods.StandbyState | spdMethods.SuspendState | spdMethods.HibernateState
                    tabStopNext: rebootButton
                    tabStopBack: logoutButton

                    onClicked: {
                        console.log("main.qml: haltRequested")
                        haltRequested()
                    }

                    onPressAndHold: {
                        if (!contextMenu) {
                            contextMenu = shutdownOptionsComponent.createObject(shutdownButton)
                            if (spdMethods.StandbyState) {
                                // 1 == Solid::PowerManagement::StandbyState
                                contextMenu.append({itemIndex: 1, itemText: i18n("Standby")})
                            }
                            if (spdMethods.SuspendState) {
                                // 2 == Solid::PowerManagement::SuspendState
                                contextMenu.append({itemIndex: 2, itemText: i18n("Suspend to RAM")})
                            }
                            if (spdMethods.HibernateState) {
                                // 3 == Solid::PowerManagement::HibernateState
                                contextMenu.append({itemIndex: 3, itemText: i18n("Suspend to Disk")})
                            }
                            contextMenu.clicked.connect(shutdownUi.suspendRequested)
                        }
                        contextMenu.open()
                    }

                    onActiveFocusChanged: {
                        shutdownUi.focusedButton = shutdownButton
                    }
                }

                Component {
                    id: shutdownOptionsComponent
                    ContextMenu {
                        visualParent: shutdownButton
                    }
                }

                KSMButton {
                    id: rebootButton
                    text: i18n("Restart Computer")
                    iconSource: "system-reboot"
                    height: 32
                    anchors.right: parent.right
                    menu: rebootOptions["options"].length > 0
                    tabStopNext: cancelButton
                    tabStopBack: shutdownButton

                    onClicked: {
                        console.log("main.qml: rebootRequested")
                        rebootRequested()
                    }

                    onPressAndHold: {
                        if (!contextMenu) {
                            contextMenu = rebootOptionsComponent.createObject(rebootButton)
                            var options = rebootOptions["options"]
                            for (var index = 0; index < options.length; ++index) {
                                var itemData = new Object
                                itemData["itemIndex"] = index
                                itemData["itemText"] = options[index]
                                if (index == rebootOptions["default"]) {
                                    itemData["itemText"] += i18nc("default option in boot loader", " (default)")
                                }
                                contextMenu.append(itemData)
                            }

                            contextMenu.clicked.connect(shutdownUi.rebootRequested2)
                        }
                        contextMenu.open()
                    }

                    onActiveFocusChanged: {
                        shutdownUi.focusedButton = rebootButton
                    }
                }

                Component {
                    id: rebootOptionsComponent
                    ContextMenu {
                        visualParent: rebootButton
                    }
                }
            }

            KSMButton {
                id: cancelButton
                anchors.right: parent.right
                text: i18n("Cancel")
                iconSource: "dialog-cancel"
                smallButton: true
                height: 22
                tabStopNext: logoutButton
                tabStopBack: rebootButton

                onClicked: {
                    cancelRequested()
                }

                onActiveFocusChanged: {
                    shutdownUi.focusedButton = cancelButton
                }
            }
        }
    }
}
