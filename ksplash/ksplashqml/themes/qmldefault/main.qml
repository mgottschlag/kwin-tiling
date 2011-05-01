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
            state1.opacity = 1
            progressBar.progress = 20
        }
        if (state == 3) {
            progressBar.progress = 40
        }
        if (state == 4) {
            state2.opacity = 0.33
            progressBar.progress = 60
        }
        if (state == 5) {
            state2.opacity = 0.67
            progressBar.progress = 80
        }
        if (state == 6) {
            state2.opacity = 1
            progressBar.progress = 100
        }
    }
    /* }}} */

    /* object properties ------------------------------{{{ */

    /* }}} */

    /* child objects ----------------------------------{{{ */
    QML.Rectangle {
        anchors.fill: parent
        gradient: QML.Gradient {
            QML.GradientStop { position: 0.0; color: "#111111" }
            QML.GradientStop { position: 0.5; color: "#222222" }
            QML.GradientStop { position: 1.0; color: "#111111" }
        }
    }

    QML.Image {
        id: state1

        height: 64
        width: 64
        smooth: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        source: "images/kde1.png"

        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
    }

    QML.Image {
        id: state2

        height: 64
        width: 64
        smooth: true

        x: (parent.width - width) / 2
        y: (parent.height - height) / 2

        source: "images/kde2.png"

        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
    }

    QML.Rectangle {
        id: progressBarBackground
        x: progressBar.x
        y: progressBar.y
        height: progressBar.height
        width: state1.width
        radius: 2

        color: "black"
    }

//    QML.Rectangle {
//        id: progressBarGlow1
//        y: progressBar.y - radius / 2
//        x: progressBar.x - radius / 2
//        height: progressBar.height + radius
//        width: progressBar.width + radius
//        radius: 4
//
//        color: "white"
//        opacity: progressBar.opacity / 4
//    }
//
//    QML.Rectangle {
//        id: progressBarGlow2
//        y: progressBar.y - radius / 2
//        x: progressBar.x - radius / 2
//        height: progressBar.height + radius
//        width: progressBar.width + radius
//        radius: 2
//
//        color: "white"
//        opacity: progressBar.opacity / 2
//    }

    QML.Rectangle {
        id: progressBar
        y: state1.y + state1.height + 8
        x: state1.x
        height: 2
        width: 0
        radius: 1

        color: "white"
        opacity: (state1.opacity + state2.opacity) / 2

        property int progress: 0

        onProgressChanged: {
            width = (state1.width * progress) / 100
        }

        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        QML.Behavior on width { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
    }

    /* }}} */

    /* states -----------------------------------------{{{ */

    /* }}} */

    /* transitions ------------------------------------{{{ */

    /* }}} */
}

