/*
 *   Copyright 2011 Marco Martin <mart@kde.org>
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

import QtQuick 1.1
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

MouseArea {
    id: main

    hoverEnabled: true
    onEntered: toolTipHideTimer.running = false
    onExited: toolTipHideTimer.running = true

    width: childrenRect.width
    height: 200

    property variant icon: tooltipDialog.appletDelegate.icon
    property string title: tooltipDialog.appletDelegate.title
    property string description: tooltipDialog.appletDelegate.description
    property string author: tooltipDialog.appletDelegate.author
    property string email: tooltipDialog.appletDelegate.email
    property string license: tooltipDialog.appletDelegate.license
    property string pluginName: tooltipDialog.appletDelegate.pluginName
    property bool local: tooltipDialog.appletDelegate.local

    QIconItem {
        id: tooltipIconWidget
        anchors.left: parent.left
        anchors.top: parent.top
        width: theme.hugeIconSize
        height: width
        icon: main.icon
    }
    Column {
        id: nameColumn
        spacing: 8
        anchors {
            left: tooltipIconWidget.right
            leftMargin: 8
            top: parent.top
            right: parent.right
        }

        PlasmaComponents.Label {
            text: title
            font.bold:true
            anchors.left: parent.left
            anchors.right: parent.right
            height: paintedHeight
            wrapMode: Text.Wrap
        }
        PlasmaComponents.Label {
            text: description
            anchors.left: parent.left
            anchors.right: parent.right
            wrapMode: Text.Wrap
        }
    }
    Grid {
        anchors.top: tooltipIconWidget.bottom
        anchors.topMargin: 16
        anchors.bottom: uninstallButton.top
        anchors.bottomMargin: 4
        rows: 2
        columns: 2
        spacing: 4
        PlasmaComponents.Label {
            text: i18n("License: ")
            wrapMode: Text.Wrap
        }
        PlasmaComponents.Label {
            id: licenseText
            text: license
            wrapMode: Text.Wrap
        }
        PlasmaComponents.Label {
            text: i18n("Author: ")
            wrapMode: Text.Wrap
        }
        PlasmaComponents.Label {
            text: author
            wrapMode: Text.Wrap
        }
    }

    PlasmaComponents.Button {
        id: uninstallButton
        anchors {
            horizontalCenter: parent.horizontalCenter
            bottom: parent.bottom
        }
        opacity: local ? 1 : 0
        Behavior on opacity {
            NumberAnimation { duration: 250 }
        }
        iconSource: "application-exit"
        text: i18n("Uninstall")
        onClicked: widgetExplorer.uninstall(pluginName)
    }
}
