import QtQuick 2.0
import Unity.Application 0.1

Rectangle {
    id: root
    color: "red"

    property alias surface: surfaceItem.surface
    property bool touchMode: false

    width: surfaceItem.width + (borderThickness*2)
    height: surfaceItem.height + titleBar.height + (borderThickness*2)

    signal cloneRequested()

    onTouchModeChanged: {
        if (touchMode) {
            x -= borderThicknessTouch - borderThicknessMouse;
            width += 2*(borderThicknessTouch - borderThicknessMouse);
            y -= borderThicknessTouch - borderThicknessMouse;
            height += 2*(borderThicknessTouch - borderThicknessMouse);
        } else {
            x += borderThicknessTouch - borderThicknessMouse;
            width -= 2*(borderThicknessTouch - borderThicknessMouse);
            y += borderThicknessTouch - borderThicknessMouse;
            height -= 2*(borderThicknessTouch - borderThicknessMouse);
        }
    }

    readonly property real minWidth: 100
    readonly property real minHeight: 100

    property real borderThickness: touchMode ? borderThicknessTouch : borderThicknessMouse
    readonly property real borderThicknessMouse: 10
    readonly property real borderThicknessTouch: 40

    states: [
        State {
            name: "closed"
            when: (surface && !surface.live) || titleBar.closeRequested
        }
    ]
    transitions: [
        Transition {
            from: ""; to: "closed"
            SequentialAnimation {
                PropertyAnimation {
                    target: root
                    property: "scale"
                    easing.type: Easing.InBack
                    duration: 400
                    from: 1.0
                    to: 0.0
                }
                ScriptAction { script: { root.destroy(); } }
            }
        }
    ]


    MouseArea {
        id: resizeArea

        anchors.fill: parent

        property real startX
        property real startY
        property real startWidth
        property real startHeight
        property bool leftBorder
        property bool rightBorder
        property bool topBorder
        property bool bottomBorder
        property bool dragging
        onPressedChanged: {
            if (pressed) {
                var pos = mapToItem(root.parent, mouseX, mouseY);
                startX = pos.x;
                startY = pos.y;
                startWidth = surfaceItem.width;
                startHeight = surfaceItem.height;
                leftBorder = mouseX > 0 && mouseX < root.borderThickness;
                rightBorder = mouseX > (root.width - root.borderThickness) && mouseX < root.width;
                topBorder = mouseY > 0 && mouseY < root.borderThickness;
                bottomBorder = mouseY > (root.height - root.borderThickness) && mouseY < root.height;
                dragging = true;
            } else {
                dragging = false;
            }
        }

        onMouseXChanged: {
            if (!pressed || !dragging) {
                return;
            }

            var pos = mapToItem(root.parent, mouseX, mouseY);

            var deltaX = pos.x - startX;
            if (leftBorder) {
                if (startWidth - deltaX >= root.minWidth) {
                    surfaceItem.surfaceWidth = startWidth - deltaX;
                } else {
                    surfaceItem.surfaceWidth = root.minWidth;
                }
            } else if (rightBorder) {
                if (startWidth + deltaX >= root.minWidth) {
                    surfaceItem.surfaceWidth = startWidth + deltaX;
                } else {
                    surfaceItem.surfaceWidth = root.minWidth;
                }
            }
        }

        onMouseYChanged: {
            if (!pressed || !dragging) {
                return;
            }

            var pos = mapToItem(root.parent, mouseX, mouseY);

            var deltaY = pos.y - startY;
            if (topBorder) {
                if (startHeight - deltaY >= root.minHeight) {
                    surfaceItem.surfaceHeight = startHeight - deltaY;
                } else {
                    surfaceItem.surfaceHeight = root.minHeight;
                }
            } else if (bottomBorder) {
                if (startHeight + deltaY >= root.minHeight) {
                    surfaceItem.surfaceHeight = startHeight + deltaY;
                } else {
                    surfaceItem.surfaceHeight = root.minHeight;
                }
            }
        }
    }

    TitleBar {
        id: titleBar
        anchors.left: parent.left
        anchors.leftMargin: root.borderThickness
        anchors.right: parent.right
        anchors.rightMargin: root.borderThickness
        anchors.top: parent.top
        anchors.topMargin: root.borderThickness

        target: root
        cloned: root.cloned
        onCloneRequested: { root.cloneRequested(); }
    }

    MirSurfaceItem {
        id: surfaceItem

        width: surface ? surface.size.width : 50
        height: surface ? surface.size.height : 50

        onWidthChanged: {
            if (resizeArea.dragging && resizeArea.leftBorder) {
                root.x = resizeArea.startX + resizeArea.startWidth - surfaceItem.width;
            }
        }

        onHeightChanged: {
            if (resizeArea.dragging && resizeArea.topBorder) {
                root.y = resizeArea.startY + resizeArea.startHeight - surfaceItem.height;
            }
        }

        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.leftMargin: root.borderThickness

        consumesInput: true
    }
}

