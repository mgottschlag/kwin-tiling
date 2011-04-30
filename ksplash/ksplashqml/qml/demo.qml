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

    width: screenSize.width
    height: screenSize.height

    /* property declarations --------------------------{{{ */
    property int state
    /* }}} */

    /* signal declarations ----------------------------{{{ */

    /* }}} */

    /* JavaScript functions ---------------------------{{{ */
    onStateChanged: {
        if (state == 2) {
            state1.x = (main.width - state1.width) / 2
            state1.opacity = 1
        }
        if (state == 3) {
            state1.x = - state1.width
            state2.x = (main.width - state2.width) / 2
            state1.opacity = 0
            state2.opacity = 1
        }
        if (state == 4) {
            state2.x = - state2.width
            state3.x = (main.width - state3.width) / 2
            state2.opacity = 0
            state3.opacity = 1
        }
        if (state == 5) {
            state3.x = - state3.width
            state4.x = (main.width - state4.width) / 2
            state3.opacity = 0
            state4.opacity = 1
        }
        if (state == 6) {
            state4.x = - state4.width
            state5.x = (main.width - state5.width) / 2
            state4.opacity = 0
            state5.opacity = 1
            state5.width = 128
            state5.height = 128
        }
    }
    /* }}} */

    /* object properties ------------------------------{{{ */

    /* }}} */

    /* child objects ----------------------------------{{{ */
    QML.Image {
        anchors.fill: parent

        source: "images/background.jpg"

        width: 1024
        height: 600
    }

    QML.Rectangle {
        width: parent.width
        height: 100
        x: 0
        y: (parent.height - height) / 2
        color: "#ffffff"
        opacity: 0.2
    }

    QML.Image {
        id: state1
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/hardware.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    QML.Image {
        id: state2
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/configuring.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    QML.Image {
        id: state3
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/globe.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    QML.Image {
        id: state4
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/desktop.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    QML.Image {
        id: state5
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/kde.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        QML.Behavior on width { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        QML.Behavior on height { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        // QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    /* }}} */

    /* states -----------------------------------------{{{ */

    /* }}} */

    /* transitions ------------------------------------{{{ */

    /* }}} */
}

