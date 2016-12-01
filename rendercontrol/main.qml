import QtQuick 2.0

Rectangle {
    width: 256
    height: 256

    gradient: Gradient {
        GradientStop { position: 0; color: "red" }
        GradientStop { position: 1; color: "#00ff00" }
    }
    border.color: "black"
    border.width: 1

    Rectangle {
        width: 64
        height: 64
        color: "blue"
        border.color: "white"
        border.width: 1
    }
}

