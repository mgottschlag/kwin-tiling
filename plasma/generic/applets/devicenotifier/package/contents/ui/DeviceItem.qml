import Qt 4.7
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    property alias deviceIcon: device.icon
    property alias deviceName: device.text
    height: device.height

    PlasmaWidgets.IconWidget {
        id: device
        anchors { top: parent.top; left: parent.left }
        drawBackground: true
        orientation: QtHorizontal
        text: dataSource.data[modelData]["text"]
        width: parent.width
    }
}
