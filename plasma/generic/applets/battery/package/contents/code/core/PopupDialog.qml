import Qt 4.7
import org.kde.plasma.core 0.1 as PlasmaCore
import org.kde.plasma.graphicswidgets 0.1 as PlasmaWidgets

Item {
    id: dialog
    width: 387
    height: 135

    property int percent
    property bool pluggedIn
    property alias screenBrightness: values.screenBrightness

    signal sleepClicked
    signal hibernateClicked
    signal changeBrightness

    Item {
        id: labels
        width: parent.width/3
        height: parent.height
        anchors {
            left: parent.left
            top: parent.top
            topMargin: 10
            bottom: parent.bottom
        }

        Text {
            id: batteryLabel
            text: "Battery:"
            anchors {
                right: parent.right
                rightMargin: 10
            }
        }

        Text {
            id: adapterLabel
            text: "AC Adapter:"
            anchors {
                top: batteryLabel.bottom
                topMargin: 10
                right: parent.right 
                rightMargin: 10
            }
        }

        Text {
            id: profileLabel
            text: "Power Profile:"
            anchors {
                top: adapterLabel.bottom
                topMargin: 10
                right: parent.right 
                rightMargin: 10
            }
        }

        Text {
            id: brightnessLabel
            text: "Screen Brightness:"
            anchors {
                top: profileLabel.bottom
                topMargin: 10
                right: parent.right 
                rightMargin: 10
            }
        }
    }

    Item {
        id: values
        anchors {
            left: labels.right
            right: parent.right
            top: parent.top
            topMargin: 10
            bottom: parent.bottom
        }
        property int screenBrightness

        Text {
            id: batteryValue
            text: percent+"%" + " (" + (percent<100 ? (pluggedIn ? "charging" : "discharging") : "charged") + ")"
            font.bold: true
            anchors {
                left: parent.left
            }
        }

        Text {
            id: adapterValue
            text: pluggedIn ? "Plugged in" : "Not plugged in"
            font.bold: true
            anchors {
                left: parent.left
                top: batteryValue.bottom
                topMargin: 10
            }
        }

        PlasmaWidgets.ComboBox {
            id: profiles
            anchors {
                left: parent.left
                top: adapterValue.bottom
                topMargin: 5
                right: parent.right
            }
        }

        PlasmaWidgets.Slider {
            id: brightnessSlider
            orientation: Qt.Horizontal
            minimum: 0
            maximum: 100
            value: parent.screenBrightness
            anchors {
                left: parent.left
                top: adapterValue.bottom
                topMargin: -5
                right: parent.right
            }
            onValueChanged: changeBrightness()
        }

        PlasmaWidgets.IconWidget {
            id: sleepButton
            icon: QIcon("system-suspend")
            drawBackground: true
            //textBackgroundColor: Qt.transparent
            textBackgroundColor: "#00000000"
            text: "Sleep"
            orientation: Qt.Horizontal
            anchors {
                left: parent.left
                top: brightnessSlider.bottom
                topMargin: -40
                bottom: parent.bottom
            }
            onClicked: sleepClicked()
        }

        PlasmaWidgets.IconWidget {
            id: hibernateButton
            icon: QIcon("system-suspend-hibernate")
            drawBackground: true
            //textBackgroundColor: Qt.transparent
            textBackgroundColor: "#00000000"
            text: "Hibernate"
            orientation: Qt.Horizontal
            anchors {
                right: parent.right
                top: brightnessSlider.bottom
                topMargin: -40
                bottom: parent.bottom
            }
            onClicked: hibernateClicked()
        }
    }
}
