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

        ListElement { icon: "system-lock-screen";       op: "lock";       show: true  }
        ListElement { icon: "system-switch-user";       op: "switchUser"; show: false }
        ListElement { icon: "system-shutdown";          op: "leave";      show: true  }
        ListElement { icon: "system-suspend";           op: "suspend";    show: false }
        ListElement { icon: "system-suspend-hibernate"; op: "hibernate";  show: false }
    }

    function updateIcons() {
        iconList.setProperty (0, "show", show_lock);
        iconList.setProperty (1, "show", show_switchUser);
        iconList.setProperty (2, "show", show_leave);
        iconList.setProperty (3, "show", show_suspend);
        iconList.setProperty (4, "show", show_hibernate);
    }

    Flow {
        id: iconView
        anchors.fill: parent
        flow: orientation==Qt.Vertical ? Flow.TopToBottom : Flow.LeftToRight
        Repeater {
            /* should be:
            model: PlasmaCore.SortFilterModel {
                sourceModel: iconList
                filterRole: "show"
                filterRegExp: "true"
            }
            // error: sourceModel expects a QAbstractItemModel
            */

            model: iconList
            delegate: Item {
                id: iconDelegate
                visible: show
                width: visible ? iconButton.width : 0
                height: visible ? iconButton.height : 0

                QIconItem {
                    id: iconButton
                    icon: QIcon(model.icon)
                    width: iconView.flow==Flow.LeftToRight ? lockout.width/count : lockout.width
                    height: iconView.flow==Flow.TopToBottom ? lockout.height/count : lockout.height
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

