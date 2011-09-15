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

import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {
    id: lockout
    property bool show_lock: true
    property bool show_switchUser: false
    property bool show_leave: true
    property bool show_suspend: false
    property bool show_hibernate: false
    property int count: 2
    property int orientation: plasmoid.formFactor==2 ? Qt.Vertical : Qt.Horizontal
    property int iconSize: 80

    PlasmaCore.DataSource {
        id: dataEngine
        engine: "powermanagement"
        connectedSources: ["PowerDevil"]
        interval: 0
    }

    Component.onCompleted: {
        plasmoid.addEventListener('ConfigChanged', configChanged);
        configChanged();
    }

    function setSize() {
        if (orientation == Qt.Vertical) {
            plasmoid.resize (iconSize, iconSize*count);
        } else {
            plasmoid.resize (iconSize*count, iconSize);
        }
    }

    function configChanged() {
        show_lock = plasmoid.readConfig("show_lock");
        show_switchUser = plasmoid.readConfig("show_switchUser");
        show_leave = plasmoid.readConfig("show_leave");
        show_suspend = plasmoid.readConfig("show_suspend");
        show_hibernate = plasmoid.readConfig("show_hibernate");
        count = show_lock+show_switchUser+show_leave+show_suspend+show_hibernate;
        setSize();
        updateIcons();
    }

    ListModel {
        id: iconList
    }

    function updateIcons() {
        iconList.clear();
        if (show_lock) {
            iconList.append ({ "icon": "system-lock-screen", "op": "lock" });
        }
        if (show_switchUser) {
            iconList.append ({ "icon": "system-switch-user", "op": "switchUser" });
        }
        if (show_leave) {
            iconList.append ({ "icon": "system-shutdown", op: "leave" });
        }
        if (show_suspend) {
            iconList.append ({ "icon": "system-suspend", "op": "suspend" });
        }
        if (show_hibernate) {
            iconList.append ({ "icon": "system-suspend-hibernate", "op": "hibernate" });
        }
    }

    Flow {
        id: iconView
        anchors.fill: parent
        flow: orientation==Qt.Vertical ? Flow.TopToBottom : Flow.LeftToRight
        Repeater {
            model: iconList
            delegate: Item {
                id: iconDelegate
                width: iconView.flow==Flow.LeftToRight ? lockout.height : lockout.width
                height: iconView.flow==Flow.TopToBottom ? lockout.width : lockout.height

                QIconItem {
                    id: iconButton
                    anchors.fill: parent
                    icon: QIcon(model.icon)
                    scale: mouseArea.pressed ? 0.9 : 1
                    smooth: true

                    MouseArea {
                        id: mouseArea
                        anchors.fill: parent
                        onClicked: clickHandler(op)
                    }
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

