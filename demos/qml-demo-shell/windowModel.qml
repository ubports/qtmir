import QtQuick 2.4
import Unity.Application 0.1
import Mir.Pointer 0.1

FocusScope {
    id: root
    focus: true

    WindowModel {
        id: windowModel;
    }

    Item {
        id: windowViewContainer
        anchors.fill: parent

        Repeater {
            model: windowModel

            delegate: MirSurfaceItem {
                id: surfaceItem
                surface: model.surface
                consumesInput: true // QUESTION: why is this non-default?
                x: surface.position.x
                y: surface.position.y
                width: surface.size.width
                height: surface.size.height
                focus: surface.focused
                visible: surface.visible

                Rectangle {
                    anchors { top: parent.bottom; right: parent.right }
                    width: childrenRect.width
                    height: childrenRect.height
                    color: surface.focused ? "red" : "lightsteelblue"
                    opacity: 0.8
                    Text {
                        text: surface.position.x + "," + surface.position.y + " " + surface.size.width + "x" + surface.size.height
                        font.pixelSize: 10
                    }
                }

                Rectangle { anchors.fill: parent; z: -1; color: "black"; opacity: 0.3 }
            }
        }
    }

    Button {
        anchors { right: parent.right; top: parent.top }
        height: 30
        width: 80
        text: "Quit"
        onClicked: Qt.quit()
    }

    WindowModelDebugView {
        anchors { right: parent.right; bottom: parent.bottom }
        model: windowModel
    }

    Text {
        anchors { left: parent.left; bottom: parent.bottom }
        text: "Move window: Ctrl+click\n
Resize window: Ctrl+Right click"
    }

    Rectangle {
        id: mousePointer
        color: "black"
        width: 6
        height: 10
        x: PointerPosition.x
        y: PointerPosition.y
    }

    MouseArea {
        anchors.fill: parent
        acceptedButtons: Qt.LeftButton | Qt.RightButton
        hoverEnabled: false
        property variant window: null
        property int initialWindowXPosition
        property int initialWindowYPosition
        property int initialWindowWidth
        property int initialWindowHeight
        property int initialMouseXPosition
        property int initialMouseYPosition
        property var action

        function moveWindowBy(window, delta) {
            window.surface.requestedPosition = Qt.point(initialWindowXPosition + delta.x,
                                                        initialWindowYPosition + delta.y);
        }
        function resizeWindowBy(window, delta) {
            window.surface.resize(Qt.size(initialWindowWidth + delta.x,
                                          initialWindowHeight + delta.y))
        }

        onPressed: {
            if (mouse.modifiers & Qt.ControlModifier) {
                window = windowViewContainer.childAt(mouse.x, mouse.y)
                if (!window) return;

                if (mouse.button == Qt.LeftButton) {
                    initialWindowXPosition = window.surface.position.x
                    initialWindowYPosition = window.surface.position.y
                    action = moveWindowBy
                } else if (mouse.button == Qt.RightButton) {
                    initialWindowHeight = window.surface.size.height
                    initialWindowWidth = window.surface.size.width
                    action = resizeWindowBy
                }
                initialMouseXPosition = mouse.x
                initialMouseYPosition = mouse.y
            } else {
                mouse.accepted = false
            }
        }

        onPositionChanged: {
            if (!window) {
                mouse.accepted = false
                return
            }
            action(window, Qt.point(mouse.x - initialMouseXPosition, mouse.y - initialMouseYPosition))
        }

        onReleased: {
            if (!window) {
                mouse.accepted = false
                return
            }
            action(window, Qt.point(mouse.x - initialMouseXPosition, mouse.y - initialMouseYPosition))
            window = null;
        }
    }
}
