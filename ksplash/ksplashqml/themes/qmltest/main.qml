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
    property int stage
    /* }}} */

    /* signal declarations ----------------------------{{{ */

    /* }}} */

    /* JavaScript functions ---------------------------{{{ */
    onStageChanged: {
        if (stage == 2) {
            stage1.x = (main.width - stage1.width) / 2
            stage1.opacity = 1
        }
        if (stage == 3) {
            stage1.x = - stage1.width
            stage2.x = (main.width - stage2.width) / 2
            stage1.opacity = 0
            stage2.opacity = 1
        }
        if (stage == 4) {
            stage2.x = - stage2.width
            stage3.x = (main.width - stage3.width) / 2
            stage2.opacity = 0
            stage3.opacity = 1
        }
        if (stage == 5) {
            stage3.x = - stage3.width
            stage4.x = (main.width - stage4.width) / 2
            stage3.opacity = 0
            stage4.opacity = 1
        }
        if (stage == 6) {
            stage4.x = - stage4.width
            stage5.x = (main.width - stage5.width) / 2
            stage4.opacity = 0
            stage5.opacity = 1
            stage5.width = 128
            stage5.height = 128
        }
    }
    /* }}} */

    /* object properties ------------------------------{{{ */

    /* }}} */

    /* child objects ----------------------------------{{{ */
    QML.Image {
        anchors.fill: parent

        source: "images/background.jpg"
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
        id: stage1
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/hardware.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    QML.Image {
        id: stage2
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/configuring.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    QML.Image {
        id: stage3
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/globe.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    QML.Image {
        id: stage4
        x: main.width + width
        y: (main.height - height) / 2
        source: "images/desktop.png"
        QML.Behavior on x { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }
        opacity: 0
        QML.Behavior on opacity { QML.NumberAnimation { duration: 1000; easing { type: QML.Easing.InOutQuad } } }

        QML.Image { y: 128; x: parent.x - main.width / 2; height: 128; width: 128; opacity: 0.2 * parent.opacity; source: parent.source }
    }

    QML.Image {
        id: stage5
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

    /* stages -----------------------------------------{{{ */

    /* }}} */

    /* transitions ------------------------------------{{{ */

    /* }}} */
}

