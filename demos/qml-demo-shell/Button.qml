import QtQuick 2.0

Rectangle {
    id: root

    signal clicked

    property alias text: text.text

    implicitHeight: 40
    implicitWidth: 150

    color: (mouseArea.pressed) ? "red" : "lightblue"

    MouseArea {
        id: mouseArea
        anchors.fill: parent
        onClicked: { root.clicked(); }
    }

    Text {
        id: text
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
    }
}
