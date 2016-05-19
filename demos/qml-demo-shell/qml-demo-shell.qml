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
        onScaleChanged: print("NOTICE: scale changed for", model.screen, "to", scale);
        onFormFactorChanged: print("NOTICE: form factor changed for", model.screen, "to", formFactor)
        Button {
            anchors { left: parent.left; bottom: parent.bottom }
            height: 100
            width: parent.width / 2
            text: "Scale up"
            onClicked: window.setScaleAndFormFactor(window.scale + 0.2, Screens.FormFactorMonitor)
        }
        Button {
            anchors { right: parent.right; bottom: parent.bottom }
            height: 100
            width: parent.width / 2
            text: "Scale down"
            onClicked: window.setScaleAndFormFactor(window.scale - 0.2, Screens.FormFactorTablet)
        }
    }
}
