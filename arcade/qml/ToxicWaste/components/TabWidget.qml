import QtQuick 1.1

Item {
    id: tabWidget

    default property alias content: stack.children
    property int current: 0
    property color baseColor: parent.color
    property color activeBorderColor: "transparent"
    property color inactiveBorderColor: "transparent"
    property color activeTextColor: "black"
    property color inactiveTextColor: "black"
    property int fontSize: 12

    onCurrentChanged: setOpacities()
    Component.onCompleted: setOpacities()

    function setOpacities() {
        for (var i = 0; i < stack.children.length; i++)
            stack.children[i].opacity = (i == current ? 1: 0);
    }

    function tabHeaderWidth() {
        return (tabWidget.width - stack.children.length - 1) / stack.children.length;
    }

    function firstKeyNavItem() {
        return stack.children[current].firstKeyNavItem;
    }

    function lastKeyNavItem() {
        return stack.children[current].lastKeyNavItem;
    }

    Row {
        id: header

        Repeater {
            model: stack.children.length
            delegate: Rectangle {
                width: tabHeaderWidth()
                x: (tabHeaderWidth() + 3) * index
                height: 30
                color: tabWidget.current == index ? baseColor : "transparent"
                border.color: tabWidget.current == index ? activeBorderColor : inactiveBorderColor
                border.width: 1
                radius: 5
                smooth: true
                Text {
                    horizontalAlignment: Qt.AlignHCenter
                    verticalAlignment: Qt.AlignVCenter
                    anchors.fill: parent
                    text: stack.children[index].title
                    elide: Text.ElideRight
                    font.bold: tabWidget.current == index
                    font.pixelSize: tabWidget.fontSize
                    color: tabWidget.current == index ? activeTextColor : inactiveTextColor
                    smooth: true
                }
                MouseArea {
                    anchors.fill: parent
                    onClicked: tabWidget.current = index
                }
            }
        }
    }

    Item {
        id: stack
        width: tabWidget.width
        anchors.top: header.bottom
        anchors.topMargin: 3
        anchors.bottom: tabWidget.bottom
    }
}
