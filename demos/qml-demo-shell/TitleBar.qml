import QtQuick 2.4
import Unity.Application 0.1

Rectangle {
    id: root
    color: "brown"
    height: 25

    property Item target
    property bool cloned: false
    property bool closeRequested: false
    signal cloneRequested()

    MouseArea {
        anchors.fill: parent
        property real distanceX
        property real distanceY
        property bool dragging
        onPressedChanged: {
            if (pressed) {
                var pos = mapToItem(root.target, mouseX, mouseY);
                distanceX = pos.x;
                distanceY = pos.y;
                dragging = true;
                Mir.cursorName = "grabbing";
            } else {
                dragging = false;
                Mir.cursorName = "";
            }
        }
        onMouseXChanged: {
            if (dragging) {
                var pos = mapToItem(root.target.parent, mouseX, mouseY);
                root.target.x = pos.x - distanceX;
                root.target.y = pos.y - distanceY;
            }
        }
    }

    Text {
        visible: !root.cloned
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        text: "CLONE"
        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.cloneRequested();
            }
        }
    }

    Text {
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        width: contentWidth
        text: "X"
        fontSizeMode: Text.VerticalFit
        minimumPixelSize: 10; font.pixelSize: 72
        font.weight: Font.Bold
        horizontalAlignment: Text.AlignRight
        verticalAlignment: Text.AlignVCenter
        MouseArea {
            anchors.fill: parent
            onClicked: {
                root.closeRequested = true;
            }
        }
    }
}

