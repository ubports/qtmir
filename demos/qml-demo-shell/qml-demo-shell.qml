import QtQuick 2.3
import Unity.Screens 0.1

Instantiator {
    id: root

    property var screens: Screens{}

    model: screens
    ScreenWindow {
        id: window
        visible: true
        screen: model.screen
        Shell{ anchors.fill: parent }
        Component.onCompleted: {
            print("Window created for Screen", screen, screen.geometry, outputType, Screens.HDMIA, screen.devicePixelRatio)
        }
        Component.onDestruction: {
            print("Window destroyed")
        }
        onScaleChanged: print("NOTICE: scale changed for", model.screen);
        Button {
            anchors { left: parent.left; bottom: parent.bottom }
            height: 100
            width: parent.width / 2
            text: "Scale up"
            onClicked: window.setScale(window.scale + 0.2)
        }
        Button {
            anchors { right: parent.right; bottom: parent.bottom }
            height: 100
            width: parent.width / 2
            text: "Scale down"
            onClicked: window.setScale(window.scale - 0.2)
        }
    }
}
