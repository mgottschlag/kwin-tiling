import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id: devicenotifier
    property bool removable
    property bool nonRemovable
    property bool all

    PlasmaCore.DataSource {
        id: dataSource
        engine: "hotplug"
        connectedSources: sources
        interval: 500
    }

    Component.onCompleted: {
        plasmoid.addEventListener ('ConfigChanged', configChanged)
    }

    function configChanged() {
        removable = plasmoid.readConfig("removableDevices")
        nonRemovable = plasmoid.readConfig("nonRemovableDevices")
        all = plasmoid.readConfig("allDevices")
    }

    Text {
        id: header
        // FIXME: The following "count" is not accessible
        // text: deviceView.deviceList.count>0 ? "Available Devices" : "No Devices Available"
        text: "Available Devices"
        anchors { top: parent.top; left: parent.left; right: parent.right }
        horizontalAlignment: Text.AlignHCenter
    }

    PlasmaWidgets.Separator {
        id: separator
        anchors { top: header.bottom; left: parent.left; right: parent.right }
        anchors { topMargin: 3 }
    }

    Column {
        id: deviceView
        anchors { top : separator.bottom; topMargin: 3 }
        Repeater {
            id: deviceList
            model: dataSource.sources
            DeviceItem {
                id: deviceItem
                width: devicenotifier.width
                deviceIcon: QIcon(dataSource.data[modelData]["icon"])
                deviceName: dataSource.data[modelData]["text"]
            }
        }
    }
}
