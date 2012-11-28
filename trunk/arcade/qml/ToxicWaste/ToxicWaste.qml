import QtQuick 1.1

Rectangle {
    id: toxic_waste_main
    width: 800
    height: 600
    gradient: Gradient {
        GradientStop {
            position: 0
            color: "#3aa82b"
        }
        GradientStop {
            position: 0.75
            color: "#ffffff"
        }
        GradientStop {
            position: 1
            color: "#000000"
        }
    }
    ListView {
        id: gamelist_view
        scale: parent.width / 800
        height: parent.height / scale - 20
        highlightRangeMode: ListView.NoHighlightRange
        snapMode: ListView.NoSnap
        interactive: true
        keyNavigationWraps: false
        anchors.verticalCenterOffset: 0
        anchors.horizontalCenterOffset: 0
        x: parent.width / scale / 2 - width / 2
        y: 10
        width: 304
        anchors.verticalCenter: parent.verticalCenter
        anchors.horizontalCenter: parent.horizontalCenter
        spacing: 10
        orientation: ListView.Vertical
        flickableDirection: Flickable.AutoFlickDirection
        smooth: true
        delegate: Item {
            id: item_delegate
            height: 64
            Item {
                Image {
                    id: gameitem_image
                    fillMode: Image.PreserveAspectFit
                    smooth: true
                    source: "images/gameitem_bg.png"
                    height: item_delegate.height
                    opacity: 0.7
                    Text {
                        id: gameitem_text
                        text: name
                        color: "black"
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.horizontalCenter: parent.horizontalCenter
                        font.bold: true
                        font.italic: true
                        font.pixelSize: parent.height - 40
                    }
                }
            }
        }
        model: ListModel {
            ListElement {
                name: "Item 1"
            }
            ListElement {
                name: "Item 2"
            }
            ListElement {
                name: "Item 3"
            }
            ListElement {
                name: "Item 4"
            }
            ListElement {
                name: "Item 5"
            }
            ListElement {
                name: "Item 6"
            }
            ListElement {
                name: "Item 7"
            }
            ListElement {
                name: "Item 8"
            }
            ListElement {
                name: "Item 9"
            }
            ListElement {
                name: "Item 10"
            }
            ListElement {
                name: "Item 11"
            }
            ListElement {
                name: "Item 12"
            }
            ListElement {
                name: "Item 13"
            }
            ListElement {
                name: "Item 14"
            }
            ListElement {
                name: "Item 15"
            }
            ListElement {
                name: "Item 16"
            }
        }
    }
}
