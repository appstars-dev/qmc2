import QtQuick 1.1

Rectangle {
    id: checkboxContainer
    property bool debug: false

    property variant text
    property bool checked: false
    smooth: true
    color: "transparent"
    property alias textSize: checkboxText.font.pixelSize
    property string textColour: "white"
    property string activeColour: "blue"
    property real opacityDiff: 0.2
    property real resetOpacity: 0
    opacity: 1.0

    signal entered();
    signal clicked();
    Component.onCompleted: {
        mouseArea.entered.connect(entered);
        mouseArea.clicked.connect(clicked);
    }

    onActiveFocusChanged: {
        debug && console.log("[checkbox] activeFocus: '" + activeFocus + "'")
        checkboxMark.focus = true;
    }
    onClicked: {
        checkboxMark.focus = true;
        checked = !checked;
    }
    MouseArea {
        id: mouseArea
        anchors.fill: parent
        hoverEnabled: true
        onEntered: {
            resetOpacity = checkboxText.opacity;
            if (checkboxText.opacity > 0 )
                checkboxText.opacity += opacityDiff;
        }
        onExited: {
            debug && console.log("[checkbox exited 1] checkboxText.opacity: '" + checkboxText.opacity + "', " +
                                                     "resetOpacity: '" + resetOpacity + "'");
            checkboxText.opacity = resetOpacity
            debug && console.log("[checkbox exited 2] checkboxText.opacity: '" + checkboxText.opacity + "', " +
                                                     "resetOpacity: '" + resetOpacity + "'");
        }
    }
    Rectangle {
        id: checkboxMark
        anchors.top: parent.top
        anchors.topMargin: 0
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        anchors.left: checkboxContainer.left
        anchors.leftMargin: 0
        border.width: activeFocus ? 2 : 0
        border.color: parent.activeColour
        smooth: true
        color: "white"
        width: 16
        radius: 2
        Image {
            id: checkboxMarkImage
            source: "qrc:../images/checkmark.png"
            smooth: true
            anchors.fill: parent
            anchors.margins: 1
            fillMode: Image.PreserveAspectFit
            visible: checkboxContainer.checked
        }
        MouseArea {
            anchors.fill: parent
            onClicked: { focus = true; checkboxContainer.clicked() }
        }
    }
    Text {
        id: checkboxText
        opacity: parent.opacity - opacityDiff
        text: parent.text
        font.pixelSize: 12
        color: activeFocus ? parent.activeColour : parent.textColour
        anchors.left: checkboxMark.right
        anchors.leftMargin: 5
        smooth: true
    }
    Keys.onPressed: {
        switch ( event.key ) {
            case Qt.Key_Enter:
            case Qt.Key_Return:
            case Qt.Key_Space: {
                if ( !(event.modifiers & Qt.AltModifier) ) {
                    clicked();
                    event.accepted = true;
                }
                break;
            }
        }
    }
}
