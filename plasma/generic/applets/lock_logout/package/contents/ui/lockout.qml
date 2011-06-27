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

    function configChanged() {
        show_lock = plasmoid.readConfig("show_lock");
        show_switchUser = plasmoid.readConfig("show_switchUser");
        show_leave = plasmoid.readConfig("show_leave");
        show_suspend = plasmoid.readConfig("show_suspend");
        show_hibernate = plasmoid.readConfig("show_hibernate");
        count = show_lock+show_switchUser+show_leave+show_suspend+show_hibernate;
    }

    Flow {
        id: iconView
        anchors.fill: parent
        flow: width>height ? Flow.LeftToRight : Flow.TopToBottom
        Repeater {
            model: [ // iconname,               operation,      visible
                [ "system-lock-screen",       "lock",          show_lock         ],
                [ "system-switch-user",       "switchUser",    show_switchUser   ],
                [ "system-shutdown",          "leave",         show_leave        ],
                [ "system-suspend",           "suspend",       show_suspend      ],
                [ "system-suspend-hibernate", "hibernate",     show_hibernate    ]
            ]
            delegate: Item {
                id: iconDelegate
                visible: modelData[2]
                width: visible ? iconButton.width : 0
                height: visible ? iconButton.height : 0

                QIconItem {
                    id: iconButton
                    icon: QIcon(modelData[0])
                    width: iconView.flow==Flow.LeftToRight ? lockout.width/count : lockout.width
                    height: iconView.flow==Flow.TopToBottom ? lockout.height/count : lockout.height

                    MouseArea {
                        anchors.fill: parent
                        onClicked: clickHandler(modelData[1])
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

