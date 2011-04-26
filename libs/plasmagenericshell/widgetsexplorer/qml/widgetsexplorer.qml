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
import org.kde.plasma.core 0.1 as PlasmaCore

Item {
    id: main
    width: 200
    height: 200
    signal addAppletRequested(string pluginName)
    signal closeRequested()

    PlasmaCore.Theme {
        id: theme
    }

    PlasmaCore.Dialog {
        id: categoriesDialog
        mainItem: Column {
            Repeater {
                model: filterModel
                delegate: Text {
                    color: theme.textColor
                    text: display
                    MouseArea {
                        anchors.fill: parent
                        onClicked: {
                            appletsModel.filterType = filterType
                            appletsModel.filterQuery = filterData
                        }
                    }
                }
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
            PlasmaWidgets.PushButton {
                id: categoryButton
                text: "Categories"
                onClicked: {
                    if (categoriesDialog.visible) {
                        categoriesDialog.visible = false
                    } else {
                        categoriesDialog.showPopup(categoryButton)
                    }
                }
            }
        }
        Row {
            anchors.right: parent.right
            spacing: 4
            PlasmaWidgets.PushButton { icon: QIcon("get-hot-new-stuff"); text: "Get new widgets"}
            PlasmaWidgets.PushButton { icon: QIcon("preferences-activities"); text: "Activities"}
            PlasmaWidgets.IconWidget {
                size: "22x"+parent.height
                icon: QIcon("window-close")
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
    PlasmaWidgets.ScrollBar {
        id: scrollBar
        orientation: "Horizontal"
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        property bool moving: false
        onSliderMoved: {
            moving = true
            list.contentX = value*10
            moving = false
        }
    }
}
