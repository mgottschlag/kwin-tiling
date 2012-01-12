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

    property int minimumWidth: cellWidth + (widgetExplorer.orientation == Qt.Horizontal ? 0 : scrollBar.width)
    property int minimumHeight: topBar.height + list.delegateHeight + (widgetExplorer.orientation == Qt.Horizontal ? scrollBar.height : 0) + 4


    PlasmaComponents.ContextMenu {
        id: categoriesDialog
        visualParent: topBar.categoryButton
    }
    Repeater {
        parent: categoriesDialog
        model: widgetExplorer.filterModel
        delegate: PlasmaComponents.MenuItem {
            text: display
            separator: model["separator"]
            onClicked: {
                var item = widgetExplorer.filterModel.get(index)

                widgetExplorer.widgetsModel.filterType = item.filterType
                widgetExplorer.widgetsModel.filterQuery = item.filterData
            }
            Component.onCompleted: {
                parent = categoriesDialog
            }
        }
    }

    PlasmaComponents.ContextMenu {
        id: getWidgetsDialog
        visualParent: topBar.getWidgetsButton
    }
    Repeater {
        parent: getWidgetsDialog
        model: widgetExplorer.widgetsMenuActions
        delegate: PlasmaComponents.MenuItem {
            icon: modelData.icon
            text: modelData.text
            separator: modelData.separator
            onClicked: modelData.trigger()
            Component.onCompleted: {
                parent = getWidgetsDialog
            }
        }
    }

    PlasmaCore.Dialog {
        id: tooltipDialog
        property Item appletDelegate

        Component.onCompleted: {
            tooltipDialog.setAttribute(Qt.WA_X11NetWmWindowTypeDock, true)
            tooltipDialog.windowFlags |= Qt.WindowStaysOnTopHint|Qt.X11BypassWindowManagerHint
        }

        onAppletDelegateChanged: {
            if (!appletDelegate) {
                toolTipHideTimer.restart()
                toolTipShowTimer.running = false
            } else {
                toolTipShowTimer.restart()
                toolTipHideTimer.running = false
            }
        }
        mainItem: Tooltip { id: tooltipWidget }
        Behavior on x {
            enabled: widgetExplorer.orientation == Qt.Horizontal
            NumberAnimation { duration: 250 }
        }
        Behavior on y {
            enabled: widgetExplorer.orientation == Qt.Vertical
            NumberAnimation { duration: 250 }
        }
    }
    Timer {
        id: toolTipShowTimer
        interval: 500
        repeat: false
        onTriggered: {
            var point = tooltipDialog.popupPosition(tooltipDialog.appletDelegate)
            tooltipDialog.x = point.x
            tooltipDialog.y = point.y
            tooltipDialog.visible = true
        }
    }
    Timer {
        id: toolTipHideTimer
        interval: 1000
        repeat: false
        onTriggered: tooltipDialog.visible = false
    }

    Loader {
        id: topBar
        property Item categoryButton
        property Item getWidgetsButton

        sourceComponent: (widgetExplorer.orientation == Qt.Horizontal) ? horizontalTopBarComponent : verticalTopBarComponent
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
            height: categoryButton.height
            Row {
                spacing: 4
                anchors {
                    left: parent.left
                    leftMargin: 1
                }
                PlasmaComponents.TextField {
                    width: list.width / Math.floor(list.width / cellWidth) - 4
                    clearButtonShown: true
                    placeholderText: i18n("Enter search term...")
                    onTextChanged: widgetExplorer.widgetsModel.searchTerm = text
                    Component.onCompleted: forceActiveFocus()
                }
                PlasmaComponents.Button {
                    id: categoryButton
                    text: "Categories"
                    onClicked: categoriesDialog.open()
                }
            }
            Row {
                anchors.right: parent.right
                spacing: 4
                PlasmaComponents.Button {
                    id: getWidgetsButton
                    iconSource: "get-hot-new-stuff"
                    text: i18n("Get new widgets")
                    onClicked: getWidgetsDialog.open()
                }

                Repeater {
                    model: widgetExplorer.extraActions.length
                    PlasmaComponents.Button {
                        iconSource: widgetExplorer.extraActions[modelData].icon
                        text: widgetExplorer.extraActions[modelData].text
                        onClicked: {
                            widgetExplorer.extraActions[modelData].trigger()
                        }
                    }
                }
                PlasmaComponents.ToolButton {
                    iconSource: "window-close"
                    onClicked: widgetExplorer.closeClicked()
                }
            }
            Component.onCompleted: {
                topBar.categoryButton = categoryButton
                topBar.getWidgetsButton = getWidgetsButton
            }
        }
    }

    Component {
        id: verticalTopBarComponent

        Column {
            anchors.top: parent.top
            anchors.left:parent.left
            anchors.right: parent.right
            spacing: 4

            PlasmaComponents.ToolButton {
                anchors.right: parent.right
                iconSource: "window-close"
                onClicked: widgetExplorer.closeClicked()
            }
            PlasmaComponents.TextField {
                anchors {
                    left: parent.left
                    right: parent.right
                }
                clearButtonShown: true
                placeholderText: i18n("Enter search term...")
                onTextChanged: widgetExplorer.widgetsModel.searchTerm = text
                Component.onCompleted: forceActiveFocus()
            }
            PlasmaComponents.Button {
                id: categoryButton
                text: "Categories"
                onClicked: categoriesDialog.open()
            }



            PlasmaComponents.Button {
                id: getWidgetsButton
                iconSource: "get-hot-new-stuff"
                text: i18n("Get new widgets")
                onClicked: getWidgetsDialog.open()
            }

            Repeater {
                model: widgetExplorer.extraActions.length
                PlasmaComponents.Button {
                    iconSource: widgetExplorer.extraActions[modelData].icon
                    text: widgetExplorer.extraActions[modelData].text
                    onClicked: {
                        widgetExplorer.extraActions[modelData].trigger()
                    }
                }
            }

            Component.onCompleted: {
                topBar.categoryButton = categoryButton
                topBar.getWidgetsButton = getWidgetsButton
            }
        }
    }

    ListView {
        id: list

        property int delegateWidth: (widgetExplorer.orientation == Qt.Horizontal) ? (list.width / Math.floor(list.width / cellWidth)) : cellWidth
        property int delegateHeight: theme.defaultFont.mSize.height * 7 - 4

        anchors {
            top: topBar.bottom
            left: parent.left
            right: widgetExplorer.orientation == Qt.Horizontal ? parent.right : scrollBar.left
            bottom: widgetExplorer.orientation == Qt.Horizontal ? scrollBar.top : parent.bottom
            leftMargin: 4
            rightMargin: 4
            bottomMargin: 4
        }

        orientation: widgetExplorer.orientation == Qt.Horizontal ? ListView.Horizontal : ListView.vertical
        snapMode: ListView.SnapToItem
        model: widgetExplorer.widgetsModel

        delegate: AppletDelegate {}
    }
    PlasmaComponents.ScrollBar {
        id: scrollBar
        orientation: widgetExplorer.orientation == Qt.Horizontal ? ListView.Horizontal : ListView.Vertical
        anchors {
            top: widgetExplorer.orientation == Qt.Horizontal ? undefined : list.top
            bottom: parent.bottom
            left: widgetExplorer.orientation == Qt.Horizontal ? parent.left : undefined
            right: parent.right
        }
        flickableItem: list
    }
}
