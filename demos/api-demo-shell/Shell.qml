import QtQuick 2.5
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import Unity.Screens 0.1
import Mir.Pointer 0.1

Rectangle {
    id: shell
    color: "lightgrey"

    Column {
        anchors {
            top: parent.top
            left: parent.left
            margins: 10
        }
        spacing: 10

        ItemSelector {
            id: currentMode
            width: 200

            model:  window.screen.availableModes

            delegate: OptionSelectorDelegate { text: String(size.width + " x " + size.height) }
            selectedIndex: window.screen.currentModeIndex
        }

        Slider {
            id: slider
            width: 200
            minimumValue: 1
            maximumValue: 4
            stepSize: 1
            value: window.screen.value
            function formatValue(v) { return v; }
        }

        Button {
            text: "Apply"
            onClicked: {
                var config = window.screen.beginConfiguration();

                config.currentModeIndex = currentMode.selectedIndex;
                config.scale = slider.value;

                window.screen.applyConfiguration(config);
            }
        }
    }

    QtObject {
        id: d
        property bool editing: false
    }

    Connections {
        target: window.screen
        onAvailableModesChanged: comboModel.reset();
    }

    Rectangle {
        id: mousePointer
        color: "black"
        width: 6
        height: 10
        x: PointerPosition.x
        y: PointerPosition.y
    }

    Button {
        anchors.bottom: parent.bottom
        anchors.right: parent.right
        anchors.margins: 10
        text: "EXIT"
        onTriggered: Qt.quit()
    }
}
