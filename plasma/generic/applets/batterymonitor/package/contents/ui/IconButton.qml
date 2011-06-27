import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.qtextracomponents 0.1

Item {
    property alias icon: iconItem.icon
    property alias iconWidth: iconItem.width
    property alias iconHeight: iconItem.height
    property alias text: buttonText.text
    property int orientation

    signal clicked()

    QIconItem {
        id: iconItem
        scale: mouseArea.pressed ? 0.9 : 1
        Component.onCompleted: {
            if (orientation==Qt.Horizontal) {
                anchors.verticalCenter = parent.verticalCenter;
                anchors.left = parent.left;
                anchors.leftMargin = 5;
            }
            else if (orientation==Qt.Vertical) {
                anchors.horizontalCenter = parent.horizontalCenter;
                anchors.top = parent.top;
                anchors.topMargin = 5;
            }
        }
    }

    Text {
        id: buttonText
        Component.onCompleted: {
            if (orientation==Qt.Horizontal) {
                anchors.verticalCenter = parent.verticalCenter;
                anchors.left = iconItem.right;
                anchors.leftMargin = 5;
            }
            else if (orientation==Qt.Vertical) {
                anchors.horizontalCenter = parent.horizontalCenter;
                anchors.top = iconItem.bottom;
                anchors.topMargin = 5;
            }
        }
    }

    PlasmaCore.FrameSvgItem {
        id: highlightItem
        anchors.fill: parent
        scale: mouseArea.pressed ? 0.95 : 1
        imagePath: "widgets/viewitem"
        prefix: "hover"
        opacity: mouseArea.containsMouse ? 1 : 0
        Behavior on opacity { NumberAnimation { duration: 150 } }
    }

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onClicked: parent.clicked()
    }
}
