import QtQuick 2.4
import Unity.Application 0.1

Rectangle {
    id: root

    focus: true
    Keys.onVolumeUpPressed: {
        console.log("\"Volume Up\" pressed");
    }
    Keys.onVolumeDownPressed: {
        console.log("\"Volume Down\" pressed");
    }

    property bool resizeModeStretch: true

    gradient: Gradient {
        GradientStop { position: 0.0; color: "lightsteelblue" }
        GradientStop { position: 1.0; color: "pink" }
    }

    property bool thumbFriendlyBorders: false

    MultiPointTouchArea {
        anchors.fill: parent
        mouseEnabled: false
        onPressed: {
            root.thumbFriendlyBorders = true;
        }
        onReleased: {
            root.thumbFriendlyBorders = false;
        }
    }

    Image {
        id: unityLogo
        source: "UnityLogo.png"
        fillMode: Image.PreserveAspectFit
        anchors.centerIn: parent
        width: 600
        height: 600

        RotationAnimation {
            id: logoAnimation
            target: unityLogo
            from: 0
            to: 359
            duration: 3000
            easing.type: Easing.Linear
            loops: Animation.Infinite
        }

        MultiPointTouchArea {
            anchors.fill: parent
            minimumTouchPoints:1
            maximumTouchPoints:1
            onPressed: {
                if (logoAnimation.paused) {
                    logoAnimation.resume();
                } else if (logoAnimation.running) {
                    logoAnimation.pause();
                } else {
                    logoAnimation.start();
                }
            }
        }
    }

    Item {
        id: windowContainer
        anchors.fill: root
    }

    Rectangle {
        id: quitButton
        width: 60
        height: 40
        color: "red"
        anchors { right: parent.right; bottom: parent.bottom }
        Text {
            anchors.centerIn: parent
            text: "Quit"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: Qt.quit()
        }
    }

    Rectangle {
        id: resizeButton
        width: 90
        height: 40
        color: "blue"
        anchors { right: quitButton.left; bottom: parent.bottom }
        Text {
            anchors.centerIn: parent
            text: root.resizeModeStretch ? "Stretch" : "Wait Resize"
            color: "white"
        }
        MouseArea {
            anchors.fill: parent
            onClicked: { root.resizeModeStretch = !root.resizeModeStretch; }
        }
    }

    Rectangle {
        width: 40
        height: 40
        color: "green"
        anchors { right: resizeButton.left; bottom: parent.bottom }
        Text {
            anchors.centerIn: parent
            text: "⟳"
            color: "white"
            font.pixelSize: 35
        }
        MouseArea {
            anchors.fill: parent
            onClicked: { root.rotation += 180; }
        }
    }

    Component {
        id: windowStretchComponent
        Window {
            x: 50
            y: 50
            //width: 200
            //height: 200
            touchMode: root.thumbFriendlyBorders

            onCloneRequested: {
                var window = windowStretchComponent.createObject(windowContainer);
                window.cloned = true;
                window.surface = surface;
            }
        }
    }

    Component {
        id: windowWaitResizeComponent
        WindowBufferSized {
            x: 50
            y: 50
            touchMode: root.thumbFriendlyBorders

            onCloneRequested: {
                var window = windowStretchComponent.createObject(windowContainer);
                window.cloned = true;
                window.surface = surface;
            }
        }
    }

    property var windowComponent: resizeModeStretch ? windowStretchComponent : windowWaitResizeComponent

    SurfaceManager {
        onSurfaceCreated: {
            print("new surface", surface.name)

            var window = windowComponent.createObject(windowContainer);
            if (!window) {
                console.warn(windowComponent.errorString());
                return;
            }

            window.surface = surface;

            openAnimation.target = window;
            openAnimation.start();
        }
    }

    NumberAnimation {
        id: openAnimation
        property: "x";
        from: root.width; to: 10;
        duration: 1200; easing.type: Easing.InOutQuad
    }
}
