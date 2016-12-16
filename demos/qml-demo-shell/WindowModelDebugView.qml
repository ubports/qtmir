import QtQuick 2.0
import Unity.Application 0.1

Column {
    id: root
    width: childrenRect.width
    height: childrenRect.height
    focus: false

    property alias model: repeater.model

    function stateString(state) {
        switch(state) {
        case Mir.HiddenState:         return "Hidden"
        case Mir.RestoredState:       return "Restored"
        case Mir.MinimizedState:      return "Minimized"
        case Mir.MaximizedState:      return "Maximized"
        case Mir.VertMaximizedState:  return "VertMax"
        case Mir.FullscreenState:     return "Fullscreen"
        case Mir.HorizMaximizedState: return "HorizMax"
        case Mir.UnknownState:        return "Unknown"
        }
        return "Invalid"
    }
    function typeString(type) {
        switch(type) {
        case Mir.UnknownType:   return "Unknown"
        case Mir.NormalType:        return "Normal"
        case Mir.UtilityType:       return "Utility"
        case Mir.DialogType:        return "Dialog"
        case Mir.GlossType:         return "Gloss"
        case Mir.FreeStyleType:     return "FreeStyle"
        case Mir.MenuType:          return "Menu"
        case Mir.InputMethodType:   return "InputMethod"
        case Mir.SatelliteType:     return "Satellite"
        case Mir.TipType:           return "Tip"
        }
        return "Invalid"
    }

    function geometryString(surface) {
        return surface.position.x + "," + surface.position.y + " " + surface.size.width + "x" + surface.size.height
    }


    Text {
        text: "Index\t\Name\tVisible\tState\tType\tGeometry"
        height: (visible) ? implicitHeight : 0
        visible: repeater.count > 0
        color: "white"

        Rectangle {
            anchors.fill: parent
            color: "blue"
            z: -1
        }
    }
    Repeater {
        id: repeater
        delegate: Text {
            text: index + "\t" + surface.name + "\t" + surface.visible + "\t"
                  + stateString(surface.state) + "\t" +  typeString(surface.type) + "\t" + geometryString(surface)
            font.bold: surface.focused

            Rectangle {
                anchors.fill: parent
                color: (index % 2) ? "white" : "lightblue"
                z: -1
            }
        }
    }
}
