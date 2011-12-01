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

import QtQuick 1.0
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1
import org.kde.qtextracomponents 0.1
//import org.kde.kwin.screenlocker 1.0 as ScreenLocker

Rectangle {
    id: shutdownUi
    property int iconSize: 128
    height: iconSize + 3 * 4 + lockScreenLabel.font.pixelSize
    width: 3 * iconSize + 4 * 4
    //color: Qt.rgba(215, 215, 215, 0.8)
    color: theme.backgroundColor

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
        anchors.fill: parent

        svg: PlasmaCore.Svg {
            imagePath: "dialogs/shutdowndialog"
        }
        elementId: "center"
    }

    Component.onCompleted: {
        //console.log("margins: " + margins.left + ", " + margins.top + ", " + margins.right + ", " + margins.bottom);

        if (background.naturalSize.width < 1) {
            background.elementId = "background"
        }
    }

    Row {
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
                    print("lockScreen button triggered");
                    lockScreenRequested();

                    // Requires new kwin's screenLocker (KDE SC 4.8)
                    //ScreenLocker.lock();
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
                    print("suspend button triggered");
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
                    print("shutdown button triggered");
                    haltRequested()
                }
            }
        }
    }
}
