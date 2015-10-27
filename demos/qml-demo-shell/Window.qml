import QtQuick 2.0
import Unity.Application 0.1

Rectangle {
    id: root
    color: "red"

    property alias surface: surfaceItem.surface
    property bool touchMode: false

    width: surfaceItem.implicitWidth + 2*borderThickness
    height: surfaceItem.implicitHeight + 2*borderThickness + titleBar.height

    signal cloneRequested()
    property bool cloned: false

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

    ResizeArea {
        anchors.fill: root
        borderThickness: root.borderThickness
        target: root
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

        anchors.top: titleBar.bottom
        anchors.left: parent.left
        anchors.leftMargin: root.borderThickness
        anchors.right: parent.right
        anchors.rightMargin: root.borderThickness
        anchors.bottom: parent.bottom
        anchors.bottomMargin: root.borderThickness

        consumesInput: !root.cloned
        surfaceWidth: root.cloned ? -1 : width
        surfaceHeight: root.cloned ? -1 : height
    }
}
