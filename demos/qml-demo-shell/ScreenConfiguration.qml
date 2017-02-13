import QtQuick 2.0
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3

Rectangle {

    width: column.width + units.gu(2)
    height: column.height + units.gu(2)

    color: "transparent"
    border.width: 1
    border.color: "black"

    property QtObject screen: null

    Column {
        id: column
        anchors.centerIn: parent
        spacing: 10

        Label {
            text: screen.name
        }

        ThinDivider {}

        Column {
            Label { text: "Used" }
            CheckBox { id: useChecked; checked: screen.used; }
        }

        Label {
            text: "Mode"
        }

        ItemSelector {
            id: currentMode
            width: units.gu(30)

            model:  screen.availableModes

            delegate: OptionSelectorDelegate { text: String(size.width + " x " + size.height + "    (" + refreshRate + "Hz)") }
            selectedIndex: screen.currentModeIndex
        }

        Label {
            text: "scale"
        }

        Slider {
            id: slider
            width: units.gu(30)
            minimumValue: 1
            maximumValue: 4
            stepSize: 1
            value: screen.scale
            function formatValue(v) { return v; }
        }

        Button {
            text: "Apply"
            onClicked: {
                var config = screen.beginConfiguration();

                config.used = useChecked.checked;
                config.currentModeIndex = currentMode.selectedIndex;
                config.scale = slider.value;

                screen.applyConfiguration(config);

                // Fix up control bindings
                usedChecked.checked = Qt.binding(function() { return screen.used; });
                currentMode.selectedIndex = Qt.binding(function() { return screen.currentModeIndex; });
                slider.value = Qt.binding(function() { return screen.scale; });
            }
        }
    }
}
