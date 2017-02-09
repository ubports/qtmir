import QtQuick 2.5
import QtQuick.Window 2.0
import Unity.Screens 0.1

Instantiator {
    id: root

    model: Screens
    ScreenWindow {
        id: window
        visibility:  Window.FullScreen
        screen: model.screen

        Shell {
            width: window.width
            height: window.height
        }
    }
}
