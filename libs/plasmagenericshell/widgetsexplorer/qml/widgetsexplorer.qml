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

import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets
import org.kde.plasma.components 0.1 as PlasmaComponents
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: main
    property int minimumWidth: 200
    property int minimumHeight: 150
    signal addAppletRequested(string pluginName)
    signal closeRequested()
    property variant extraActions

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaComponents.ContextMenu {
        id: categoriesDialog
        visualParent: categoryButton
        model: filterModel
        onTriggeredIndex: {
            var item = filterModel.get(index)

            appletsModel.filterType = item.filterType
            appletsModel.filterQuery = item.filterData
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
                return
            }
            var point = tooltipDialog.popupPosition(appletDelegate)
            tooltipDialog.x = point.x
            tooltipDialog.y = point.y
            tooltipDialog.visible = true
        }
        mainItem: Tooltip { id: tooltipWidget }
        Behavior on x {
            NumberAnimation { duration: 250 }
        }
    }
    Timer {
        id: toolTipHideTimer
        interval: 1000
        repeat: false
        onTriggered: tooltipDialog.visible = false
    }

    Item {
        id: topBar
        anchors.top: parent.top
        anchors.left:parent.left
        anchors.right: parent.right
        height: categoryButton.height
        Row {
            spacing: 4
            PlasmaWidgets.LineEdit {
                width: list.width / Math.floor(list.width / 180)
                height: categoryButton.height
                onTextChanged: appletsModel.searchTerm = text
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
                iconSource: "get-hot-new-stuff"
                text: i18n("Get new widgets")
            }

            Repeater {
                model: extraActions.length
                PlasmaComponents.Button {
                    iconSource: extraActions[modelData].icon
                    text: extraActions[modelData].text
                    onClicked: {
                        extraActions[modelData].trigger()
                    }
                }
            }
            PlasmaComponents.ToolButton {
                iconSource: "window-close"
                onClicked: main.closeRequested()
            }
        }
    }
    ListView {
        id: list
        anchors.topMargin: 4
        anchors.top: topBar.bottom
        anchors.left:parent.left
        anchors.right: parent.right
        anchors.bottom: scrollBar.top
        clip: true
        orientation: ListView.Horizontal
        snapMode: ListView.SnapToItem
        model: appletsModel

        onContentXChanged: {
            if (!scrollBar.moving) {
                scrollBar.value = contentX/10
            }
        }

        onContentWidthChanged: {
            if (!scrollBar.moving) {
                scrollBar.minimum = 0
                scrollBar.maximum = (contentWidth - width)/10
            }
        }

        delegate: AppletDelegate {}
    }
    PlasmaComponents.ScrollBar {
        id: scrollBar
        orientation: Qt.Horizontal
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        flickableItem: list
    }
}
