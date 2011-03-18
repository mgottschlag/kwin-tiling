// -*- coding: iso-8859-1 -*-
/*
 *   Author: Marco Martin <mart@kde.org>
 *   Date: Fri Dec 10 2010, 23:38:41
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
import org.kde.plasma.graphicslayouts 4.7 as GraphicsLayouts

Item {
    width: 200
    height: 200

    ListModel {
        id: appletsModel
        ListElement {
            name: "Notes"
            description: "Desktop Sticky notes"
            icon: "knotes"
        }
        ListElement {
            name: "Weather"
            description: "Weather station"
            icon: "weather-clear"
        }
        ListElement {
            name: "Application Launcher menu"
            description: "Traditional menu based application launcher"
            icon: "start-here-kde"
        }
        ListElement {
            name: "Weather"
            description: "Weather station"
            icon: "weather-clear"
        }
        ListElement {
            name: "Notes"
            description: "Desktop Sticky notes"
            icon: "knotes"
        }
        ListElement {
            name: "Weather"
            description: "Weather station"
            icon: "weather-clear"
        }
        ListElement {
            name: "Notes"
            description: "Desktop Sticky notes"
            icon: "knotes"
        }
        ListElement {
            name: "Weather"
            description: "Weather station"
            icon: "weather-clear"
        }
        ListElement {
            name: "Notes"
            description: "Desktop Sticky notes"
            icon: "knotes"
        }
        ListElement {
            name: "Weather"
            description: "Weather station"
            icon: "weather-clear"
        }
        ListElement {
            name: "Notes"
            description: "Desktop Sticky notes"
            icon: "knotes"
        }
        ListElement {
            name: "Weather"
            description: "Weather station"
            icon: "weather-clear"
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
            }
            PlasmaWidgets.PushButton { id: categoryButton; text: "Category"}
        }
        Row {
            anchors.right: parent.right
            spacing: 4
            PlasmaWidgets.PushButton { icon: QIcon("get-hot-new-stuff"); text: "Get new widgets"}
            PlasmaWidgets.PushButton { icon: QIcon("preferences-activities"); text: "Activities"}
            PlasmaWidgets.IconWidget { size: "22x"+parent.height; icon: QIcon("window-close")}
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
        
        delegate: PlasmaCore.FrameSvgItem {
            id: background
            width: list.width / Math.floor(list.width / 180)
            height: list.height
            imagePath: "widgets/tasks"
            prefix: "normal"
            PlasmaWidgets.IconWidget {
                id:iconWidget
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.leftMargin: background.margins.left
                anchors.topMargin: background.margins.top
                anchors.bottomMargin: background.margins.bottom
                width: Math.min(64, height)
                icon: QIcon(model.icon)
            }
            Column {
                anchors.left: iconWidget.right
                anchors.top: parent.top
                anchors.right: parent.right
                anchors.bottom: parent.bottom
                anchors.leftMargin: background.margins.left
                anchors.topMargin: background.margins.top
                anchors.rightMargin: background.margins.right
                anchors.bottomMargin: background.margins.bottom
                Text {
                    id: titleText
                    text: name
                    font.bold:true
                    anchors.left: parent.left
                    anchors.right: parent.right
                    height: paintedHeight
                    clip: true
                    wrapMode: Text.Wrap
                }
                Text {
                    text: description
                    anchors.top: titleText.bottom
                    anchors.bottom: parent.bottom
                    anchors.left: parent.left
                    anchors.right: parent.right
                    //verticalAlignment: Text.AlignVCenter
                    clip:true
                    wrapMode: Text.Wrap
                }
            }
        }
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
