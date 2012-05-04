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
import org.kde.draganddrop 1.0
import org.kde.qtextracomponents 0.1

PlasmaCore.FrameSvgItem {
    id: background

    width: list.delegateWidth
    height: list.delegateHeight

    imagePath: "widgets/tasks"
    prefix: "normal"

    property variant icon: decoration
    property string title: name
    property string description: model.description
    property string author: model.author
    property string email: model.email
    property string license: model.license
    property string pluginName: model.pluginName
    property bool local: model.local

    ListView.onRemove: SequentialAnimation {
        PropertyAction {
            target: background
            property: "ListView.delayRemove"
            value: true
        }
        NumberAnimation {
            target: background
            property: widgetExplorer.orientation == Qt.Horizontal ? "y" : "x"
            to: widgetExplorer.orientation == Qt.Horizontal ? background.height : background.width
            duration: 250
            easing.type: Easing.InOutQuad
        }
        PropertyAction {
            target: background
            property: "ListView.delayRemove"
            value: false
        }
    }

    ListView.onAdd: NumberAnimation {
            target: background
            property: widgetExplorer.orientation == Qt.Horizontal ? "y" : "x"
            from: widgetExplorer.orientation == Qt.Horizontal ? -background.height : -background.width
            to: 0
            duration: 250
            easing.type: Easing.InOutQuad
        }

    DragArea {
        anchors.fill: parent
        supportedActions: Qt.MoveAction | Qt.LinkAction
        onDragStarted: tooltipDialog.visible = false
        mimeData {
            source: parent
        }
        Component.onCompleted: mimeData.setData("text/x-plasmoidservicename", pluginName)

        QIconItem {
                id: iconWidget
                anchors.verticalCenter: parent.verticalCenter
                x: y
                width: theme.hugeIconSize
                height: width
                icon: background.icon
            }
        Column {
            anchors {
                left: iconWidget.right
                right: parent.right
                verticalCenter: parent.verticalCenter

                leftMargin: background.margins.left
                rightMargin: background.margins.right
            }

            PlasmaComponents.Label {
                id: titleText
                text: title
                anchors {
                    left: parent.left
                    right: parent.right
                }
                height: paintedHeight
                wrapMode: Text.WordWrap
                //go with nowrap only if there is a single word too long
                onPaintedWidthChanged: {
                    wrapTimer.restart()
                }
                Timer {
                    id: wrapTimer
                    interval: 200
                    onTriggered: {
                        //give it some pixels of tolerance
                        if (titleText.paintedWidth > titleText.width + 3) {
                            titleText.wrapMode = Text.NoWrap
                            titleText.elide = Text.ElideRight
                        } else {
                            titleText.wrapMode = Text.WordWrap
                            titleText.elide = Text.ElideNone
                        }
                    }
                }
            }
            PlasmaComponents.Label {
                text: description
                font.pointSize: theme.smallestFont.pointSize
                anchors {
                    left: parent.left
                    right: parent.right
                }
                elide: Text.ElideRight
            }
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onDoubleClicked: widgetExplorer.addApplet(pluginName)
            onEntered: tooltipDialog.appletDelegate = background
            onExited: tooltipDialog.appletDelegate = null
        }
    }
}
