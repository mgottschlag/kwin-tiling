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

Item {
    id: main

    //this is used to perfectly align the filter field and delegates
    property int cellWidth: theme.defaultFont.mSize.width * 20

    property int minimumWidth: cellWidth + (activityManager.orientation == Qt.Horizontal ? 0 : scrollBar.width)
    property int minimumHeight: topBar.height + list.delegateHeight + (activityManager.orientation == Qt.Horizontal ? scrollBar.height : 0) + 4


    PlasmaCore.DataSource {
        id: activitySource
        engine: "org.kde.activities"
        onSourceAdded: {
            if (source != "Status") {
                connectSource(source)
            }
        }
        Component.onCompleted: {
            connectedSources = sources.filter(function(val) {
                return val != "Status";
            })
        }
    }

    PlasmaComponents.ContextMenu {
        id: newActivityMenu
        visualParent: topBar.newActivityButton
        PlasmaComponents.MenuItem {
            id: templatesItem
            text: i18n("Templates")
            onClicked: activityTemplatesMenu.open()
        }
        PlasmaComponents.MenuItem {
            icon: QIcon("user-desktop")
            text: i18n("Empty Desktop")
            onClicked: activityManager.createActivity("desktop")
        }
        PlasmaComponents.MenuItem {
            icon: QIcon("edit-copy")
            text: i18n("Clone current activity")
            onClicked: activityManager.cloneCurrentActivity()
        }
    }


    PlasmaComponents.ContextMenu {
        id: activityTemplatesMenu
        visualParent: templatesItem
    }
    Repeater {
        parent: activityTemplatesMenu
        model: activityManager.activityTypeActions
        delegate: PlasmaComponents.MenuItem {
            icon: QIcon(modelData.icon)
            text: modelData.text
            separator: modelData.separator
            onClicked: {
                //is a plugin?
                if (modelData.pluginName) {
                    activityManager.createActivity(modelData.pluginName)
                //is a script?
                } else if (modelData.scriptFile) {
                    activityManager.createActivityFromScript(modelData.scriptFile,  modelData.text, modelData.icon, modelData.startupApps)
                //invoke ghns
                } else {
                    activityManager.downloadActivityScripts()
                }
            }
            Component.onCompleted: {
                parent = activityTemplatesMenu
            }
        }
    }


    Loader {
        id: topBar
        property string query
        property Item newActivityButton

        sourceComponent: (activityManager.orientation == Qt.Horizontal) ? horizontalTopBarComponent : verticalTopBarComponent
        height: item.height + 2
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right

            margins: 4
        }
    }
    Component {
        id: horizontalTopBarComponent
        Item {
            anchors {
                top: parent.top
                left:parent.left
                right: parent.right
            }
            height: filterField.height

            PlasmaComponents.TextField {
                id: filterField
                anchors {
                    left: parent.left
                    leftMargin: 2
                }
                width: list.width / Math.floor(list.width / cellWidth) - 4
                clearButtonShown: true
                onTextChanged: topBar.query = text
                placeholderText: i18n("Enter search term...")
                Component.onCompleted: forceActiveFocus()
            }

            Row {
                anchors.right: parent.right
                spacing: 4
                PlasmaComponents.Button {
                    id: newActivityButton
                    iconSource: "list-add"
                    text: i18n("Create activity...")
                    onClicked: newActivityMenu.open()
                }
                PlasmaComponents.Button {
                    iconSource: "plasma"
                    text: i18n("Add widgets")
                    onClicked: activityManager.addWidgetsRequested()
                }
                PlasmaComponents.ToolButton {
                    iconSource: "window-close"
                    onClicked: activityManager.closeClicked()
                }
            }
            Component.onCompleted: {
                topBar.newActivityButton = newActivityButton
            }
        }
    }
    Component {
        id: verticalTopBarComponent
        Column {
            spacing: 4
            anchors {
                top: parent.top
                left:parent.left
                right: parent.right
            }

            PlasmaComponents.ToolButton {
                anchors.right: parent.right
                iconSource: "window-close"
                onClicked: activityManager.closeClicked()
            }

            PlasmaComponents.TextField {
                id: filterField
                anchors {
                    left: parent.left
                    right: parent.right
                }
                clearButtonShown: true
                onTextChanged: topBar.query = text
                placeholderText: i18n("Enter search term...")
                Component.onCompleted: forceActiveFocus()
            }

            PlasmaComponents.Button {
                id: newActivityButton
                iconSource: "list-add"
                text: i18n("Create activity...")
                onClicked: newActivityMenu.open()
            }
            PlasmaComponents.Button {
                iconSource: "plasma"
                text: i18n("Add widgets")
                onClicked: activityManager.addWidgetsRequested()
            }
            Component.onCompleted: {
                topBar.newActivityButton = newActivityButton
            }
        }
    }

    ListView {
        id: list

        property int delegateWidth: (activityManager.orientation == Qt.Horizontal) ? (list.width / Math.floor(list.width / cellWidth)) : cellWidth
        property int delegateHeight: theme.defaultFont.mSize.height * 7 - 4


        anchors {
            top: topBar.bottom
            left: parent.left
            right: activityManager.orientation == Qt.Horizontal ? parent.right : scrollBar.left
            bottom: activityManager.orientation == Qt.Horizontal ? scrollBar.top : parent.bottom
            leftMargin: 4
            rightMargin: 4
            bottomMargin: 4
        }

        orientation: activityManager.orientation == Qt.Horizontal ? ListView.Horizontal : ListView.vertical
        snapMode: ListView.SnapToItem
        model: PlasmaCore.SortFilterModel {
            sourceModel: PlasmaCore.DataModel {
                dataSource: activitySource
            }
            filterRole: "Name"
            filterRegExp: ".*"+topBar.query+".*"
        }

        delegate: ActivityDelegate {}
    }
    PlasmaComponents.ScrollBar {
        id: scrollBar
        orientation: activityManager.orientation == Qt.Horizontal ? ListView.Horizontal : ListView.Vertical
        anchors {
            top: activityManager.orientation == Qt.Horizontal ? undefined : list.top
            bottom: parent.bottom
            left: activityManager.orientation == Qt.Horizontal ? parent.left : undefined
            right: parent.right
        }
        flickableItem: list
    }
}
