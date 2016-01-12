import QtQuick 2.3
import QtQuick.Window 2.2 as QQW
import Unity.Screens 0.1

Instantiator {
    id: root

    property var screens: Screens{}

    model: screens
    QQW.Window {
        id: window
        visible: true
        Shell{ anchors.fill: parent }
        Component.onCompleted: {
            print("HEY", screen, screen.geometry, outputType, Screens.HDMIA, screen.devicePixelRatio)
        }
    }
}
