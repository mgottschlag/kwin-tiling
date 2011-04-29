/*   vim:set foldenable foldmethod=marker:
 *
 *   Copyright (C) 2011 Ivan Cukic <ivan.cukic(at)kde.org>
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License version 2,
 *   or (at your option) any later version, as published by the Free
 *   Software Foundation
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details
 *
 *   You should have received a copy of the GNU General Public
 *   License along with this program; if not, write to the
 *   Free Software Foundation, Inc.,
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

import Qt 4.7 as QML

QML.Item {
    id: main

    width: 1024
    height: 600

    /* property declarations --------------------------{{{ */
    property int state
    /* }}} */

    /* signal declarations ----------------------------{{{ */

    /* }}} */

    /* JavaScript functions ---------------------------{{{ */
    onStateChanged: {
        // text.text = "new state: " + state
        if (state == 2) {
            state1.x = - state1.width
            state2.x = (main.width - state2.width) / 2
        }
        if (state == 3) {
            state2.x = - state2.width
            state3.x = (main.width - state3.width) / 2
        }
        if (state == 4) {
            state3.x = - state3.width
            state4.x = (main.width - state4.width) / 2
        }
        if (state == 5) {
            state4.x = - state4.width
            state5.x = (main.width - state5.width) / 2
        }
    }
    /* }}} */

    /* object properties ------------------------------{{{ */

    /* }}} */

    /* child objects ----------------------------------{{{ */
    QML.Image {
        anchors.fill: parent

        source: "/home/ivan/.kde/share/wallpapers/stripes-debian-blue/contents/images/1024x768.jpg"

        width: 1024
        height: 600
    }

    QML.Image {
        id: state1
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/hardware.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000 } }
    }

    QML.Image {
        id: state2
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/configuring.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000 } }
    }

    QML.Image {
        id: state3
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/globe.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000 } }
    }

    QML.Image {
        id: state4
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/desktop.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000 } }
    }

    QML.Image {
        id: state5
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/kde.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000 } }
    }


    /* }}} */

    /* states -----------------------------------------{{{ */

    /* }}} */

    /* transitions ------------------------------------{{{ */

    /* }}} */
}

