import QtQuick 2.5
import Ubuntu.Components 1.3
import Ubuntu.Components.ListItems 1.3
import Unity.Screens 0.1

Instantiator {
    id: root

    model: Screens

    ScreenWindow {
        id: window
        visible: true
        screen: model.screen

        Column {
            x: 10
            y: 10
            spacing: 10

            ItemSelector {
                id: currentMode
                width: 200

                model:  window.screen.availableModes

                delegate: OptionSelectorDelegate { text: String(size.width + " x " + size.height + "    (" + refreshRate + "Hz)") }
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

        Shell {
            width: parent.width
            height: parent.height
        }
    }
}
