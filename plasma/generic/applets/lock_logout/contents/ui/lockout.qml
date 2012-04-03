/*
 *   Copyright 2011 Viranch Mehta <viranch.mehta@gmail.com>
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
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Flow {
    id: lockout
    property int minimumWidth
    property int minimumHeight

    property bool show_lock: true
    property bool show_switchUser: false
    property bool show_leave: true
    property bool show_suspend: false
    property bool show_hibernate: false
    property int myCount: 2
    property int orientation: plasmoid.formFactor<2 ? (width<height ? Qt.Vertical : Qt.Horizontal) : (plasmoid.formFactor==2 ? Qt.Horizontal : Qt.Vertical)

    flow: orientation==Qt.Vertical ? Flow.TopToBottom : Flow.LeftToRight
    clip: true

    // when panel is resized
    onHeightChanged: {
        if (plasmoid.formFactor==2) {
            minimumWidth = width = height*myCount;
        }
    }
    onWidthChanged: {
        if (plasmoid.formFactor==3) {
            minimumHeight = height = width*myCount;
        }
    }

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
    }

    Component.onCompleted: {
        plasmoid.aspectRatioMode = 0;
        plasmoid.addEventListener('ConfigChanged', configChanged);
    }

    // resizes the applet whenever the list of icons to be shown changes
    // NOTE: this function should be called BEFORE updating the value of
    // myCount. Icons auto-resize according to value of myCount. We need
    // to first resize the applet before icons auto-resize.
    // The new value of the count should be passed to the function, and
    // then assign it to myCount AFTER the function returns.
    function setSize(count) {
        // resize the applet only if it is on a panel
        // otherwise, the icons resize themselves
        if (plasmoid.formFactor<2) {
            return;
        }

        if (orientation == Qt.Vertical) { // vertical panel
            height = minimumHeight = items.iconSize*count;
        } else { // horizontal panel
            width = minimumWidth = items.iconSize*count;
        }
    }

    function configChanged() {
        show_lock = plasmoid.readConfig("show_lock");
        show_switchUser = plasmoid.readConfig("show_switchUser");
        show_leave = plasmoid.readConfig("show_leave");
        show_suspend = plasmoid.readConfig("show_suspend");
        show_hibernate = plasmoid.readConfig("show_hibernate");

        var newCount = show_lock+show_switchUser+show_leave+show_suspend+show_hibernate;
        setSize(newCount);
        myCount = newCount;

        iconList.get(0).to_show = show_lock;
        iconList.get(1).to_show = show_switchUser;
        iconList.get(2).to_show = show_leave;
        iconList.get(3).to_show = show_suspend;
        iconList.get(4).to_show = show_hibernate;
    }

    Repeater {
        id: items
        property int itemWidth: parent.flow==Flow.LeftToRight ? parent.width/myCount : parent.width
        property int itemHeight: parent.flow==Flow.TopToBottom ? parent.height/myCount : parent.height
        property int iconSize: Math.min(itemWidth, itemHeight)

        model: ListModel {
            id: iconList
            ListElement { icon: "system-lock-screen"; op: "lock"; to_show: true }
            ListElement { icon: "system-switch-user"; op: "switchUser"; to_show: false }
            ListElement { icon: "system-shutdown"; op: "leave"; to_show: true }
            ListElement { icon: "system-suspend"; op: "suspend"; to_show: false }
            ListElement { icon: "system-suspend-hibernate"; op: "hibernate"; to_show: false }
        }

        delegate: Item {
            id: iconDelegate
            visible: model.to_show
            width: items.itemWidth
            height: items.itemHeight

            QIconItem {
                id: iconButton
                width: items.iconSize
                height: items.iconSize
                anchors.centerIn: parent
                icon: QIcon(model.icon)
                scale: mouseArea.pressed ? 0.9 : 1

                MouseArea {
                    id: mouseArea
                    anchors.fill: parent
                    onClicked: clickHandler(op)
                }
            }
        }
    }

    function clickHandler(what) {
        var service = dataEngine.serviceForSource("PowerDevil");
        var operation = service.operationDescription(what);
        service.startOperationCall(operation);
    }
}

