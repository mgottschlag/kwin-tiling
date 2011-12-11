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
import org.kde.plasma.graphicswidgets 0.1
import org.kde.qtextracomponents 0.1

PlasmaCore.FrameSvgItem {
    id: shutdownUi
    property int iconSize: 128
    property int realMarginTop: margins.top
    property int realMarginBottom: margins.bottom
    property int realMarginLeft: margins.left
    property int realMarginRight: margins.right
    width: realMarginLeft + iconRow.width + realMarginRight
    height: realMarginTop + iconRow.height + realMarginBottom

    imagePath: "dialogs/shutdowndialog"

    signal logoutRequested()
    signal haltRequested()
    signal suspendRequested(int spdMethod)
    signal rebootRequested()
    signal rebootRequested2(int opt)
    signal cancelRequested()
    signal lockScreenRequested()

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
    }

    Row {
        id: iconRow
        spacing: 5
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        Column {
            Text {
                id: lockScreenLabel
                text: i18n("lock")
                anchors.horizontalCenter: lockScreenIcon.horizontalCenter
                color: theme.textColor
                font.pixelSize: 18
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
            IconWidget {
                id: lockScreenIcon
                icon: QIcon("system-lock-screen")
                minimumIconSize: "128x128"

                onClicked: {
                    lockScreenRequested();
                }
            }
        }

        Column {
            Text {
                text: i18n("sleep")
                anchors.horizontalCenter: sleepIcon.horizontalCenter
                color: theme.textColor
                font.pixelSize: 18
                font.family: theme.defaultFont.family
                font.bold: theme.defaultFont.bold
                font.capitalization: theme.defaultFont.capitalization
                font.italic: theme.defaultFont.italic
                font.weight: theme.defaultFont.weight
                font.underline: theme.defaultFont.underline
                font.wordSpacing: theme.defaultFont.wordSpacing
            }
            IconWidget {
                id: sleepIcon
                icon: QIcon("system-suspend")
                minimumIconSize: "128x128"

                onClicked: {
                    suspendRequested(2); // Solid::PowerManagement::SuspendState
                }
            }
        }

        Column {
            Text {
                text: i18n("turn off")
                anchors.horizontalCenter: shutdownIcon.horizontalCenter
                color: theme.textColor
                font.pixelSize: 18
                font.family: theme.defaultFont.family
                font.bold: theme.defaultFont.bold
                font.capitalization: theme.defaultFont.capitalization
                font.italic: theme.defaultFont.italic
                font.weight: theme.defaultFont.weight
                font.underline: theme.defaultFont.underline
                font.wordSpacing: theme.defaultFont.wordSpacing
            }
            IconWidget {
                id: shutdownIcon
                icon: QIcon("system-shutdown")
                minimumIconSize: "128x128"

                onClicked: {
                    haltRequested()
                }
            }
        }
    }
}
