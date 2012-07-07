/*
 *   Copyright 2012 Gregor Taetzner <gregor@freenet.de>
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
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id: iconContainer
    property string activeSource: "Status"
    property int minimumWidth: 16
    property int minimumHeight: 16

    Component.onCompleted: {
        plasmoid.aspectRatioMode = "ConstrainedSquare"
    }

    PlasmaCore.DataSource {
        id: dataSource
        engine: "org.kde.activities"
        connectedSources: [activeSource]
    }

    PlasmaCore.ToolTip {
        id: tooltip
        mainText: i18n("Show Activity Manager")
        subText: i18n("Click to show the activity manager")
        target: icon
        image: "preferences-activities"
    }

    PlasmaWidgets.IconWidget
    {
        id: icon
        svg: "widgets/activities"
        anchors.fill: parent
        onClicked:
        {
            var service = dataSource.serviceForSource(activeSource)
            var operation = service.operationDescription("toggleActivityManager")
            service.startOperationCall(operation)
        }
    }
}

