import QtQuick 2.0;
import QtQuick.Controls 1.0;
import "./components";
import "darkone.js" as DarkoneJS;

Rectangle {
    id: darkone
    z: 0

    property bool initialised: false
    property int fps: 0
    property bool ignoreLaunch: false
    property bool dataHidden: true
    property bool keepLightOn: false
    property bool lightOut: true
    property bool lightOutScreen: true
    property string dataTypeCurrent: "title"
    property int zoomDuration: 250
    property int listDuration: 750
    property int flashes: 4
    property int overlayDuration: 0
    property int toolbarHideIn: 0
    property bool preferencesLaunchLock: false
    property bool toolbarShowMenuLock: false
    property bool toolbarShowFpsLock: false
    property bool infoMissing: true
    property real backLightOpacity: 0
    property string colour1: "#000000"
    property string colour2: "#000000"
    property string colour3: "#000000"
    property string colour4: "#000000"
    property string colour5: "#000000"
    property string textColour1: "#000000"
    property string textColour2: "#000000"
    property string version: ""
    property bool debug: false

    // restored properties
    property int lastIndex: -1
    property bool toolbarHidden: false
    property bool listHidden: false
    property bool fullScreen: false
    property bool fpsVisible: false
    property bool sortByName: false
    property bool launchFlash: true
    property bool launchZoom: true
    property string dataTypePrimary: ""
    property string dataTypeSecondary: ""
    property real lightTimeout: 60
    property bool backLight: true
    property bool toolbarAutoHide: true
    property real overlayScale: 1
    property string colourScheme: "dark"

    width: DarkoneJS.baseWidth
    height: DarkoneJS.baseHeight
    color: "black"
    opacity: 0
    state: "off"

    PropertyAnimation { id: fadeIn; target: darkone; property: "opacity"; duration: 2000; from: 0; to: 1.0; easing.type: Easing.InExpo; }
    onToolbarAutoHideChanged: { debug && console.log("toolbarAutoHide: '" + toolbarAutoHide + "'"); }
    onLastIndexChanged: { debug && console.log("lastIndex: '" + lastIndex + "'"); }
    onColourSchemeChanged: { DarkoneJS.colourScheme(colourScheme); }
    Component.onCompleted: initTimer.start()
    Connections {
        target: viewer;
        onEmulatorStarted: DarkoneJS.gameOn();
        onEmulatorFinished: DarkoneJS.gameOver();
    }
    onStateChanged: state == "off" ? lightOffAnimation.start() : lightOnAnimation.start()
    SequentialAnimation {
        id: lightOnAnimation
        PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 150; from: 0; to: 1.0; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 75; from: 1.0; to: 0.25; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 75; from: 0.25; to: 1.0; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 75; from: 1.0; to: 0.25; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 105; from: 0.25; to: 1.0; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 250; from: 1.0; to: 0.75; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 0; from: 0.75; to: 1.0; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayStateBlock; property: "opacity"; duration: 5; from: 0.0; to: 1.0; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayStateBlock; property: "opacity"; duration: 5; from: 0; to: 0.5; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayTextFlick; property: "opacity"; duration: 5; from: 0; to: 1.0; easing.type: Easing.InExpo; }
    }
    SequentialAnimation {
        id: lightOffAnimation
        PropertyAnimation { target: overlayStateBlock; property: "opacity"; duration: 0; from: overlayStateBlock.opacity; to: 0; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayCabinet; property: "opacity"; duration: 5; from: 1.0; to: 0.1; easing.type: Easing.InExpo; }
        PropertyAnimation { target: overlayTextFlick; property: "opacity"; duration: 5; from: 1.0; to: 0; easing.type: Easing.InExpo; }
    }
    onLightOutChanged: { debug && console.log("[darkone] lightOut: '" + lightOut + ", " +
                                                        "state before: '" + darkone.state + "'");
                         lightOut ? darkone.state = "off" : darkone.state = "on"; }
    onLightOutScreenChanged: { debug && console.log("[darkone] lightOutScreen: '" + lightOutScreen + ", " +
                                                              "overlayScreen.state orig: '" + overlayScreen.state + "', " +
                                                              "backLightOpacity: '" + backLightOpacity + "'");
                              if (lightOutScreen) {
                                  backLightOpacity = 0;
                                  overlayScreen.state = "off"
                               } else {
                                  if (backLight)
                                     backLightOpacity = 1.0;
                                  overlayScreen.state = "on";
                               } }
    onDataHiddenChanged: { dataHidden ? overlayData.state = "hidden" : overlayData.state = "shown"; }
    onInfoMissingChanged: { infoMissing ? overlayText.state = "missing" : overlayText.state = "found"; }
    onOverlayScaleChanged: { overlayScaleSliderItem.value = overlayScale; }
    onFullScreenChanged: {
        if ( !DarkoneJS.initialising ) {
            if ( fullScreen ) {
                viewer.switchToFullScreen();
                fullScreenToggleButton.state = "fullscreen";
            } else {
                viewer.switchToWindowed();
                fullScreenToggleButton.state = "windowed";
            }
        }
    }

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPositionChanged: { if (!keepLightOn) {
                                 lightOutTimer.restart();
                                 lightOutScreenTimer.restart();
                             }
                             if (lightOut)
                                 DarkoneJS.lightToggle(1);
        }
    }

    Timer {
        id: initTimer
        interval: 5
        running: false
        repeat: true
        onTriggered: DarkoneJS.init()
    }
    Timer {
        id: resetIgnoreLaunchTimer
        interval: 100
        running: false
        repeat: false
        onTriggered: darkone.ignoreLaunch = false
    }
    Timer {
        //reset overlay width to snap instantantly
        id: resetOverlaySnapTimer
        interval: 1000
        repeat: false
        onTriggered: { darkone.overlayDuration = 0; }
    }
    Timer {
        id: launchFlashTimer
        interval: 750
        running: false
        repeat: true
        onTriggered: DarkoneJS.launchDelay()
    }
    Timer {
        id: launchTimer
        interval: 250
        running: false
        repeat: true
        onTriggered: { if (!overlayScreenScaleAnimation.running) {
                          launchTimer.stop();
                          viewer.launchEmulator(gameListModel[gameListView.currentIndex].id);
                       } } }
    Timer {
        id: hideToolbarTimer
        interval: 500
        running: false
        repeat: true
        onTriggered: {
            if (toolbarHideIn == 0) {
                hideToolbarTimer.stop();
                toolbarHideIn = 3;
                toolbarAutoHide && DarkoneJS.toolbarToggle(-1);
            } else
                toolbarHideIn -= 1;
        }
    }
    Timer {
        id: lightOutTimer
        interval: lightTimeout * 1000
        running: false
        repeat: true
        onTriggered: DarkoneJS.lightToggle(-1);
    }
    Timer {
        id: lightOutScreenTimer
        interval: lightTimeout * 1000 + 2500
        running: false
        repeat: true
        onTriggered: { lightOutScreen = true;
                       lightOutScreenTimer.stop(); }
    }

//FocusScope {

/***
* overlay
*/
    Rectangle {
        id: overlay
        z: 0
        opacity: debug ? 0.25 : 1.0
        width: (DarkoneJS.overlayWidth() - 15 - 15)
        border.color: debug ? "red" : "transparent"
        border.width: debug ? 2 : 0
        color: debug ? "white" : "transparent"
        anchors.top: parent.top
        anchors.topMargin: 10
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 10
        anchors.right: parent.right
        anchors.rightMargin: 15
        Behavior on width { PropertyAnimation { duration: overlayDuration; easing.type: Easing.InOutQuad } }
        MouseArea {
            anchors.fill: parent
            onClicked: { debug && console.log("[overlay] clicked");
                         searchTextInput.focus = false; }
            onWheel: {
                       DarkoneJS.zoom(1 + (0.1) * (wheel.angleDelta.y / Math.abs(wheel.angleDelta.y)));
                       debug && console.log("[overlay] wheel event: overlayScale: '" + overlayScale + "', " +
                                                                   "zoom: '" + (1 + (0.1) * (wheel.angleDelta.y / Math.abs(wheel.angleDelta.y))) + "'");
            }
        }


/***
* screen
*/
        Rectangle {
            id: overlayScreen
            z: 1
            width: 348
            height: 256
            scale: DarkoneJS.scaleFactorY() * overlayScale
            anchors.top: parent.top
            // keep the screen still under scaling, ensure margin of 30% of non-screen space
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (height * scale))
            anchors.horizontalCenter: parent.horizontalCenter
            smooth: true
            opacity: 1.0
            border.color: debug ? "blue" : "transparent"
            border.width: debug ? 2 : 0
            color: debug ? "white" : "#181818"
            state: "on"
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onDoubleClicked: { debug && console.log("[overlayScreen] double-clicked");
                                   dataHidden = !dataHidden; }
                onEntered: { debug && console.log("[overlayScreen] entered"); }
                onPositionChanged: { if (!keepLightOn) {
                                         lightOutTimer.restart();
                                         lightOutScreenTimer.restart();
                                     }
                                     if (lightOut)
                                         DarkoneJS.lightToggle(1);
                }
            }
            transitions: [
                Transition {
                    from: "on"
                    to: "off"
                    ParallelAnimation {
                        PropertyAnimation { target: overlayScreen; property: "opacity"; from: 1.0; to: 0.1; duration: 500; easing.type: Easing.OutExpo }
                        PropertyAnimation { target: overlayLighting; property: "opacity"; from: backLightOpacity; to: 0; duration: 500; easing.type: Easing.OutExpo }

                    }
                },
                Transition {
                    from: "off"
                    to: "on"
                    ParallelAnimation {
                        PropertyAnimation { target: overlayScreen; property: "opacity"; from: 0.1; to: 1.0; duration: 500; easing.type: Easing.OutExpo }
                        PropertyAnimation { target: overlayLighting; property: "opacity"; from: 0; to: backLightOpacity; duration: 500; easing.type: Easing.OutExpo }
                    }
                }
            ]
            Behavior on scale { PropertyAnimation { id: "overlayScreenScaleAnimation"; properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
            onStateChanged: { debug && console.log("[overlayScreen] state changed, state: '" + state + "', " +
                                                                   "backLightOpacity: '" + backLightOpacity + "'"); }


/***
* display
*/
            Rectangle {
                id: overlayDisplay
                z: 2 // must be above other layers (in the same hierarchy!) for flickable actions
                width: parent.width
                height: parent.height
                anchors.fill: parent
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.horizontalCenterOffset: 0
                opacity: 1.0
                smooth: true
                state: "shown"
                color: "transparent"
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.CrossCursor
                    onDoubleClicked: { debug && console.log("[overlayDisplay] double-clicked");
                                       dataHidden = !dataHidden; }
                }
                Behavior on anchors.topMargin { PropertyAnimation { duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                Behavior on anchors.bottomMargin { PropertyAnimation { duration: darkone.zoomDuration; easing.type: Easing.Linear } }

                Image {
                    id: overlayImage
                    z: 0
                    source: DarkoneJS.data("image")
                    smooth: true
                    anchors.fill: parent
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.centerIn: parent
                    fillMode: Image.PreserveAspectFit
                    MouseArea {
                        anchors.fill: parent
                        onDoubleClicked: { debug && console.log("[overlayImage] double-clicked");
                                           dataHidden = !dataHidden; }
                        onWheel: {
                            DarkoneJS.zoom(1 + (0.1) * (wheel.angleDelta.y / Math.abs(wheel.angleDelta.y)));
                        }
                    }
                }

                Rectangle {
                    id: overlayTextWrap
                    z: 1
                    color: debug ? "blue" : "transparent"
                    anchors.fill: parent
                    height: parent.height
                    width: parent.width

                    Flickable {
                        id: overlayTextFlick
                        interactive: true
                        contentHeight: Math.max(parent.height, overlayText.paintedHeight)  // no flickable without this set. vertical alignment dependent on this also
                        anchors.fill: parent
                        anchors.topMargin: 5
                        anchors.bottomMargin: 5
                        anchors.leftMargin: 8
                        anchors.rightMargin: 8
                        anchors.verticalCenter: parent.verticalCenter
                        boundsBehavior: Flickable.StopAtBounds
                        flickableDirection: Flickable.VerticalFlick
                        clip: true
                        maximumFlickVelocity : 100
                        MouseArea {
                            anchors.fill: parent
                            onDoubleClicked: { debug && console.log("[overlayTextFlick] double-clicked");
                                               dataHidden = !dataHidden; }
                            onWheel: {
                                DarkoneJS.zoom(1 + (0.1) * (wheel.angleDelta.y / Math.abs(wheel.angleDelta.y)));
                            }
                        }
                        Text {
                            id: overlayText
                            anchors.fill: parent
                            text: DarkoneJS.data("text")
                            color: "white"
                            font.bold: false
                            font.pixelSize: 8
                            verticalAlignment: Text.AlignTop
                            horizontalAlignment: Text.AlignLeft
                            wrapMode: Text.WordWrap
                            smooth: true
                            state: "found"
                            states: [
                                State {
                                    name: "found"
                                    PropertyChanges { target: overlayText; verticalAlignment: Text.AlignTop }
                                    PropertyChanges { target: overlayText; horizontalAlignment: Text.AlignLeft }
                                },
                                State {
                                    name: "missing"
                                    PropertyChanges { target: overlayText; verticalAlignment: Text.AlignVCenter }
                                    PropertyChanges { target: overlayText; horizontalAlignment: Text.AlignHCenter }
                                }
                            ]
                        }
                    }
                }
            } // overlayDisplay

            Rectangle {
                id: overlayData
                z: 1
                width: parent.width
                height: parent.height
                anchors.fill: parent
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.horizontalCenterOffset: 0
                opacity: 1.0
                state: "hidden"
                color: "transparent"
                MouseArea {
                    cursorShape: Qt.CrossCursor
                }
                transitions: [
                    Transition {
                        from: "hidden"
                        to: "shown"
                        SequentialAnimation {
                            ParallelAnimation
{
                                PropertyAnimation { target: overlayDisplay; property: "anchors.topMargin"; from: 0; to: overlayDataHeader.height; duration: 450; easing.type: Easing.InExpo }
                                PropertyAnimation { target: overlayDataHeader; property: "anchors.topMargin"; from: -overlayDataHeader.height - 5; to: 0; duration: 450; easing.type: Easing.InExpo }
                                PropertyAnimation { target: overlayDataHeader; property: "opacity"; from: 0; to: 1.0; duration: 450; easing.type: Easing.InExpo }

                                PropertyAnimation { target: overlayDisplay; property: "anchors.bottomMargin"; from: 0; to: overlayDataNav.height; duration: 450; easing.type: Easing.InExpo }
                                PropertyAnimation { target: overlayDataNav; property: "anchors.bottomMargin"; from: -overlayDataNav.height - 5; to: 0; duration: 450; easing.type: Easing.InExpo }
                                PropertyAnimation { target: overlayDataNav; property: "opacity"; from: 0; to: 1.0; duration: 450; easing.type: Easing.OutExpo }

                            }
                        }
                    },
                    Transition {
                        from: "shown"
                        to: "hidden"
                        SequentialAnimation {
                            ParallelAnimation {

                                PropertyAnimation { target: overlayDisplay; property: "anchors.topMargin"; from: overlayDataHeader.height; to: 0; duration: 450; easing.type: Easing.OutExpo }
                                PropertyAnimation { target: overlayDataHeader; property: "anchors.topMargin"; from: 0; to: -overlayDataHeader.height - 5; duration: 450; easing.type: Easing.OutExpo }
                                PropertyAnimation { target: overlayDataHeader; property: "opacity"; from: 1.0; to: 0; duration: 450; easing.type: Easing.OutExpo }

                                PropertyAnimation { target: overlayDisplay; property: "anchors.bottomMargin"; from: overlayDataNav.height; to: 0; duration: 450; easing.type: Easing.OutExpo }
                                PropertyAnimation { target: overlayDataNav; property: "anchors.bottomMargin"; from: 0; to: -overlayDataNav.height - 5; duration: 450; easing.type: Easing.OutExpo }
                                PropertyAnimation { target: overlayDataNav; property: "opacity"; from: 1.0; to: 0; duration: 450; easing.type: Easing.OutExpo }

                            }
                        }
                    }
                ]
                Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                Rectangle {
                    id: overlayDataHeader
                    height: overlayDataTitle.paintedHeight + 10
                    width: parent.width
                    anchors.top: parent.top
                    anchors.horizontalCenter: parent.horizontalCenter
                    opacity: 0
                    smooth: true
                    color: debug ? "yellow" : "transparent"
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { dataHidden = !dataHidden; }
                    }
                    Text {
                          id: overlayDataTitle
                          text: DarkoneJS.gameCardHeader()
                          font.pixelSize: 10
                          anchors.top: parent.top
                          anchors.topMargin: 5
                          anchors.horizontalCenter: parent.horizontalCenter
                          width: parent.width - 20
                          horizontalAlignment: Text.AlignHCenter
                          verticalAlignment: Text.AlignVCenter
                          color: "white"
                          wrapMode: Text.WordWrap
                          onTextChanged: { debug &&  console.log("[overlayDataHeader] changed");
                                           parent.height = paintedHeight + 5;  // force update
                                           if (!dataHidden)
                                               overlayDisplay.anchors.topMargin = paintedHeight + 5
                                         } // force update
                    }
                }
                Rectangle {
                    id: overlayDataNav
                    height: 30
                    width: parent.width
                    anchors.horizontalCenter: parent.horizontalCenter
                    anchors.bottom: parent.bottom
                    color: debug ? "red" : parent.color
                    opacity: 0
                    smooth: true
                    MouseArea {
                        anchors.fill: parent
                        onClicked: { debug && console.log("[overlayDataNav] clicked"); }
                    }

                    Rectangle {
                        id: overlayDataNavSeparator
                        height: 1
                        width: parent.width * 0.5
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.top: parent.top
                        anchors.topMargin: 5
                        color: "white"
                        opacity: 0.75
                        smooth: true
                    }
                    Rectangle {
                        id: overlayDataTypeSetPrimaryButton
                        height: 14
                        width: 10
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 5
                        anchors.left: parent.left
                        anchors.leftMargin: 10
                        color: "white"
                        border.width: 2
                        border.color: "black"
                        opacity: 0.75
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.CrossCursor
                            onClicked: { darkone.dataTypePrimary = darkone.dataTypeCurrent;
                                         debug && console.log("[overlayDataTypeSetPrimaryButton clicked]"); }
                        }
                        Text {
                            id: overlayDataTypeSetPrimaryText
                            text: "1"
                            color: "black"
                            font.bold: true
                            font.pixelSize: 8
                            anchors.fill: parent
                            anchors.centerIn: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            smooth: true
                        }
                    }
                    Rectangle {
                        id: overlayDataTypeSetSecondaryButton
                        height: 14
                        width: 10
                        anchors.bottom: parent.bottom
                        anchors.bottomMargin: 5
                        anchors.left: overlayDataTypeSetPrimaryButton.right
                        anchors.leftMargin: 3
                        color: "white"
                        border.width: 2
                        border.color: "black"
                        opacity: 0.75
                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.CrossCursor
                            onClicked: { darkone.dataTypeSecondary = darkone.dataTypeCurrent;
                                         debug && console.log("[overlayDataTypeSetSecondaryButton clicked]"); }
                        }
                        Text {
                            id: overlayDataTypeSetSecondaryText
                            text: "2"
                            color: "black"
                            font.bold: true
                            font.pixelSize: 8
                            anchors.fill: parent
                            anchors.centerIn: parent
                            horizontalAlignment: Text.AlignHCenter
                            verticalAlignment: Text.AlignVCenter
                            smooth: true
                        }
                    }
                    Image {
                        id: overlayDataTypePreviousButton
                        opacity: 0.5
                        height: 12
                        anchors.top: parent.top
                        anchors.topMargin: 5 + 1 + (parent.height - 5 - 1 - height) / 2
                        anchors.left: parent.left
                        anchors.leftMargin: parent.width / 4
                        source: "images/arrow.png"
                        mirror: true
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onContainsMouseChanged: containsMouse ? parent.opacity = 1.0 : parent.opacity = 0.75
                            onClicked: {
                                dataTypeCurrent = DarkoneJS.adjDataType(dataTypeCurrent, -1);
                                searchTextInput.focus = false;
                            }
                        }
                    }
                    Text {
                        id: overlayDataTypeText
                        text: DarkoneJS.data("name")
                        color: "white"
                        font.bold: true
                        font.pixelSize: 12
                        anchors.fill: parent
                        anchors.topMargin: 5 + 1
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        verticalAlignment: Text.AlignVCenter
                        horizontalAlignment: Text.AlignHCenter
                        smooth: true
                    }
                    Image {
                        id: overlayDataTypeNextButton
                        opacity: 0.5
                        height: 12
                        anchors.top: parent.top
                        anchors.topMargin: 5 + 1 + (parent.height - 5 - 1 - height) / 2
                        anchors.right: parent.right
                        anchors.rightMargin: parent.width / 4
                        source: "images/arrow.png"
                        fillMode: Image.PreserveAspectFit
                        smooth: true
                        MouseArea {
                            anchors.fill: parent
                            hoverEnabled: true
                            onContainsMouseChanged: containsMouse ? parent.opacity = 1.0 : parent.opacity = 0.75
                            onClicked: { dataTypeCurrent = DarkoneJS.adjDataType(dataTypeCurrent, 1); }
                        }
                    }
                }
            } // overlayData
        } // overlayScreen


/***
* cabinet
*/
        Image {
            id: overlayCabinet
            z: 2
            source: "images/cabinet.png"
            fillMode: Image.PreserveAspectFit
            scale: DarkoneJS.scaleFactorY() * overlayScale
            anchors.top: parent.top
            //keep the cabinet still under scaling, then shift it by the same amount as the screen is shift, then offet the image to match the screen
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) - (559 * scale)
            anchors.horizontalCenter: overlayScreen.horizontalCenter
            anchors.horizontalCenterOffset: 3 + 2 * overlayScale
            smooth: true
            opacity: 1.0
            Behavior on scale { ParallelAnimation {
                                  PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear }
                                  PropertyAnimation { target: overlayLighting; properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } } }
        }
        Image {
            z: 0
            id: overlayLighting
            source: "images/backlight.png"
            fillMode: Image.PreserveAspectFit
            scale: overlayCabinet.scale
            anchors.top: parent.top
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) - (559 * scale)
            anchors.horizontalCenter: overlayScreen.horizontalCenter
            anchors.horizontalCenterOffset: overlayCabinet.anchors.horizontalCenterOffset
            smooth: true
            opacity: backLight ? backLightOpacity : 0
        }
        Rectangle {
            id: overlayStateBlock
            z: 0
            width: 150
            height: 275
            scale: DarkoneJS.scaleFactorY() * overlayScale
            anchors.top: parent.top
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) + (300 * scale)
            anchors.horizontalCenter: overlayCabinet.horizontalCenter
            anchors.horizontalCenterOffset: 0
            color: DarkoneJS.gameStatusColour()
            smooth: true
            opacity: 0.0
            Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
        }
        Rectangle {
            id: overlayButtonBlock
            z: 2
            width: 25
            height: 35
            scale: DarkoneJS.scaleFactorY() * overlayScale
            anchors.top: parent.top
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) + (319 * scale)
            anchors.horizontalCenter: overlayCabinet.horizontalCenter
            anchors.horizontalCenterOffset: 2 * overlayScale
            color: debug ? "white" : "transparent"
            Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.CrossCursor
                hoverEnabled: true
                onEntered: { overlayStateBlock.opacity = (lightOnAnimation.running || lightOut) ? 0 : 1.0; }
                onExited: { overlayStateBlock.opacity = (lightOnAnimation.running || lightOut) ? 0 : 0.5; }
                onClicked: { if (!ignoreLaunch) {
                                 gameListView.positionViewAtIndex(lastIndex, ListView.Center);
                                 DarkoneJS.launch(); }
                }
            }
        }
        Rectangle {
            id: overlayGridBlock
            z: 2
            width: 120
            height: 40
            scale: DarkoneJS.scaleFactorY() * overlayScale
            anchors.top: parent.top
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) + (500 * scale)
            anchors.horizontalCenter: overlayCabinet.horizontalCenter
            anchors.horizontalCenterOffset: 1 * overlayScale
            color: debug ? "white" : "transparent"
            Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.CrossCursor
                hoverEnabled: true
                onEntered: { overlayStateBlock.opacity = (lightOnAnimation.running || lightOut) ? 0 : 1.0; }
                onExited: { overlayStateBlock.opacity = (lightOnAnimation.running || lightOut) ? 0 : 0.5; }
                onClicked: { if (!ignoreLaunch) {
                                 gameListView.positionViewAtIndex(lastIndex, ListView.Center);
                                 DarkoneJS.launch(); }
                }
            }
        }
    } // overlay

/***
* list
*/
    ListView {
        id: gameListView
        property int itemHeight: 24
        z: 3
        height: parent.height - (toolbarHidden ? 2 : toolbar.height) - gameListView.itemHeight - gameListView.itemHeight
        width: DarkoneJS.listWidth()
        anchors.top: parent.top
        anchors.topMargin: gameListView.itemHeight
        anchors.left: parent.left
        anchors.leftMargin: 15
        model: gameListModel
        spacing: 10
        clip: true
        orientation: ListView.Vertical
        flickableDirection: Flickable.VerticalFlick
        flickDeceleration: 1000
        maximumFlickVelocity: 5000
        interactive: true
        keyNavigationWraps: false
        currentIndex: darkone.lastIndex || 0
        preferredHighlightBegin: (height / 2) - (gameListView.itemHeight / 2)
        preferredHighlightEnd: (height / 2) + (gameListView.itemHeight / 2)
        states: [
            State {
                name: "hidden"
                PropertyChanges { target: showListButton; anchors.left: toolbar.left; }
                PropertyChanges { target: gameListView; anchors.leftMargin: -DarkoneJS.listWidth() - 5 }
                PropertyChanges { target: gameListView; opacity: 0 }
                PropertyChanges { target: searchBox; opacity: 0 }
            },
            State {
                name: "shown"
                PropertyChanges { target: showListButton; anchors.left: searchBox.right; }
                PropertyChanges { target: gameListView; anchors.leftMargin: 15 }
                PropertyChanges { target: gameListView; opacity: 1.0 }
                PropertyChanges { target: searchBox; opacity: 1.0 }
            }
        ]
        transitions: [
            //note: here we jump through hoops for an issue where by if a list is hidden in non-fullscreen, and
            //then fullscreen is enabled, the leftMargin isn't updated and so the (hidden) list is partially viewable.
            //the clean way is to switch anchors from 'left against parent.left' to 'right against parent.left',
            //but setting left/right anchors to 'undefined' in a state doesn't seem to work. the setting doesn't
            //take effect, and hence the list is squashed out of existence instead of moved which looks bad
            //a massive negitive margin would work fine but that's a very bad hack. so as it stands here, opacity
            //comes to the rescue. it seems natural to set hidden elements to transparent, but just remember the above
            //issue means that the hidden list can become technically unhidden (unhidden but invisible)
            Transition {
                from: "hidden"
                to: "shown"
                SequentialAnimation {
                    // ensure correct initial position
                    PropertyAnimation { target: gameListView; property: "anchors.leftMargin"; from: anchors.leftMargin; to: -DarkoneJS.listWidth() - 5; duration: 0; easing.type: Easing.Linear }
                    PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: anchors.leftMargin; to: -DarkoneJS.listWidth() - 5; duration: 0; easing.type: Easing.Linear }
                    // make visible (set in the state property)
                    PropertyAnimation { target: gameListView; property: "opacity"; duration: 0; }
                    PropertyAnimation { target: searchBox; property: "opacity"; duration: 0; }
                    // re-anchor show/hide list button (set in the state property)
                    PropertyAnimation { target: showListButton; property: "anchors.left"; duration: 0; }
                    // animate
                    ParallelAnimation {
                        PropertyAnimation { target: gameListView; property: "anchors.leftMargin"; from: -DarkoneJS.listWidth() - 5; to: 15; duration: listDuration; easing.type: Easing.InOutQuad }

                        PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: -DarkoneJS.listWidth() - 5; to: 15; duration: listDuration; easing.type: Easing.InOutQuad } } } },
            Transition {
                from: "shown"
                to: "hidden"
                SequentialAnimation {
                    // ensure correct initial position
                    PropertyAnimation { target: gameListView; property: "anchors.leftMargin"; from: anchors.leftMargin; to: DarkoneJS.listWidth() +15; duration: 0; easing.type: Easing.Linear }
                    PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: anchors.leftMargin; to: DarkoneJS.listWidth() + 15; duration: 0; easing.type: Easing.Linear }
                    // animate
                    ParallelAnimation {
                        PropertyAnimation { target: gameListView; property: "anchors.leftMargin"; from: 15; to: -DarkoneJS.listWidth() - 5; duration: listDuration; easing.type: Easing.InOutQuad }
                        PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: 15; to: -DarkoneJS.listWidth() - 5; duration: listDuration; easing.type: Easing.InOutQuad } }
                    // make invisible (set in the state property)
                    PropertyAnimation { target: gameListView; property: "opacity"; duration: 0; }
                    PropertyAnimation { target: searchBox; property: "opacity"; duration: 0; }
                    // re-anchor show/hide list button (set in the state property)
                    PropertyAnimation { target: showListButton; property: "anchors.left"; duration: 0; }
             } }
        ]
        onCurrentIndexChanged: { darkone.lastIndex = currentIndex; }

        /* item */
        delegate: Component {
            id: gameListItemDelegate
            Rectangle {
                width: parent.width
                height: gameListView.itemHeight
                id: gameListItemBackground
                smooth: true
                border.color: "#333333"
                border.width: 1
                gradient: Gradient {
                              GradientStop { position: 0.0; color: colour1 }
                              GradientStop { position: 0.2; color: colour2 }
                              GradientStop { position: 0.7; color: colour3 }
                              GradientStop { position: 1.0; color: colour4 } }
                opacity: 0.75
                Text {
                    property bool fontResized: false
                    id: gameListItemText
                    anchors.fill: parent
                    anchors.margins: 10
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: model.modelData.description
                    color: gameListItemBackground.ListView.isCurrentItem ? textColour2: textColour1
                    font.bold: true
                    font.pixelSize: 13
                    elide: Text.ElideRight
                    smooth: true
                }
                MouseArea {
                    id: gameListItemMouseArea
                    anchors.fill: parent
                    hoverEnabled: true
                    acceptedButtons: Qt.LeftButton
                    onContainsMouseChanged: {
                        if ( mapToItem(toolbar, mouseX, mouseY).y < 0 ) {
                            if ( containsMouse )
                                DarkoneJS.itemEntered(gameListItemText, gameListItemBackground, mapToItem(toolbar, mouseX, mouseY));
                            else
                                DarkoneJS.itemExited(gameListItemText, gameListItemBackground, mapToItem(toolbar, mouseX, mouseY));
                        }
                    }
                    onDoubleClicked: { if (!ignoreLaunch) {
                                          gameListView.currentIndex = index;
                                          gameListView.positionViewAtIndex(lastIndex, ListView.Center) ;
                                          DarkoneJS.launch(); }
                    }
                    onClicked: {
                        gameListView.currentIndex = index;
                        debug && console.log("[gameListView] setting index: '" + index + "'")
                        searchTextInput.focus = false;
                    }
                }
            }
        }

        function firstVisibleItem() { return - Math.floor(((height / 2) / (gameListView.itemHeight + 10))); } // relatives 'work'
        function lastVisibleItem() { return + Math.floor(((height / 2) / (gameListView.itemHeight + 10))); } // relatives 'work'
        function itemsPerPage() { debug && console.log("contentX: '" + contentX + "', " +
                                                       "contentY: '" + contentY + "', " +
                                                       "firstVisibleItem: '" + firstVisibleItem() + "', " +
                                                       "lastVisibleItem: '" + lastVisibleItem() + "', " +
                                                       "itemsPerPage: '" + height / (gameListView.itemHeight + 10) + "'");
                                           return lastVisibleItem() - firstVisibleItem() + 1 }
        function listUp() {
            if ( currentIndex - (itemsPerPage() - 1) > 0 ) {
                currentIndex -= (itemsPerPage() - 1)
                gameListView.positionViewAtIndex(currentIndex, ListView.Contain);
            } else {
                gameListView.positionViewAtBeginning();
                currentIndex = 0;
            }
        }
        function listDown() {
            if ( currentIndex + (itemsPerPage() - 1) < gameListModelCount ) {
                currentIndex += (itemsPerPage() - 1)
                gameListView.positionViewAtIndex(currentIndex, ListView.Contain);
            } else {
                gameListView.positionViewAtEnd();
                currentIndex = gameListModelCount - 1;
            }
        }

        Keys.onPressed: {
            if (!keepLightOn) {
                lightOutTimer.restart();
                lightOutScreenTimer.restart();
            }
            if (lightOut)
                DarkoneJS.lightToggle(1);
            switch ( event.key ) {
                case Qt.Key_PageUp: {
                    listUp();                    
                    event.accepted = true;
                    break;
                }
                case Qt.Key_PageDown: {
                    listDown();                    
                    event.accepted = true;
                    break;
                }
                case Qt.Key_Home: {
                    positionViewAtBeginning();
                    currentIndex = 0;
                    event.accepted = true;
                    break;
                }
                case Qt.Key_End: {
                    positionViewAtEnd();
                    currentIndex = gameListModelCount - 1;
                    event.accepted = true;
                    break;
                }
                case Qt.Key_Enter:
                case Qt.Key_Return: {
                    if ( !searchTextInput.focus && !(event.modifiers & Qt.AltModifier) && !ignoreLaunch ) {
                        gameListView.positionViewAtIndex(lastIndex, ListView.Center);
                        DarkoneJS.launch();
                    }
                    break;
                }
                default: {
                    if ( event.modifiers & Qt.ControlModifier ) { 
                        if ( event.modifiers & Qt.ShiftModifier ) {
                            switch ( event.key ) {
                                case Qt.Key_Up: {
                                    positionViewAtBeginning();
                                    currentIndex = 0;
                                    event.accepted = true;
                                    break;
                                }
                                case Qt.Key_Down: {
                                    positionViewAtEnd();
                                    currentIndex = gameListModelCount - 1;
                                    event.accepted = true;
                                    break;
                                }
                            }
                        } else {
                            switch ( event.key ) {
                                case Qt.Key_Up: {
                                    listUp();                    
                                    event.accepted = true;
                                    break;
                                }
                                case Qt.Key_Down: {
                                    listDown();                    
                                    event.accepted = true;
                                    break;
                                }
                            }
                        }
                    }
                }
            }
        }
    }


/***
* preferences menu
*/

    FocusScope {

    id: preferencesFocusScope
    focus: true
//    x: preferencesDialog.x;
//    y: preferencesDialog.y;
    x: preferencesButton.x - 10
    y: parent.height - toolbar.height - 5 - height
    width: preferencesDialog.width;
    height: preferencesDialog.height;

    Rectangle {
        id: preferencesDialog
        z: 4
        property int itemHeight: 12
        property int itemSpacing: 6
        property int itemTextSize: 9
        smooth: true
//        x: preferencesButton.x - 10
//        y: parent.height - toolbar.height - 5 - height
        width: 175
        height: (itemHeight + itemSpacing) * 19 + 10
        border.color: "transparent"
        border.width: 1
        color: colour5
        opacity: 1.0
        state: "hidden"
        focus: false
        MouseArea {
            anchors.fill: parent;
            hoverEnabled: true
            onClicked: {
                debug && console.log("[preferences onClick 1] focus: '" + focus + "'");
                focus = true;
                debug && console.log("[preferences onClick 2] focus: '" + focus + "'"); }
        }
        onFocusChanged: {
            debug && console.log("[preferences onFocus] focus: '" + focus + "'");
            border.color = debug && activeFocus ? "green" : "transparent";
            darkone.focus = !focus;
        }
        onStateChanged: {
            if ( state == "shown" ) {
                ignoreLaunch = true;
                preferencesLaunchLock = true;
                sortByName.focus = true;
                toolbarShowMenuLock = true;
                overlayScaleSliderItem.maximum = DarkoneJS.overlayScaleMax * 1.5;
                overlayScaleSliderItem.minimum = DarkoneJS.overlayScaleMin;
                overlayScaleSliderItem.value = overlayScale;
            } else {
                preferencesLaunchLock = false;
                ignoreLaunch = false;
                toolbarShowMenuLock = false;
                focus = false;
            }
        }
        states: [
            State {
                name: "hidden"
                PropertyChanges { target: preferencesDialog; opacity: 0.0 }
            },
            State {
                name: "shown"
                PropertyChanges { target: preferencesDialog; opacity: 1.0 }
            }
        ]
        transitions: Transition {
            from: "hidden"
            to: "shown"
            reversible: true
            PropertyAnimation { property: "opacity"; duration: 100 }
        }
        Text {
            id: headerText
            property int index: 1
            text: qsTr("Preferences")
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing) - parent.itemSpacing
            anchors.left: parent.left
            anchors.leftMargin: 10
            font.pixelSize: parent.itemTextSize + 3
            font.bold: true
            color: textColour1
            smooth: true
        }

        /**********
        * behaviour
        */
        Text {
            id: prefsText
            property int index: 2
            opacity: 1.0
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            text: qsTr("behaviour")
            font.pixelSize: parent.itemTextSize + 2
            font.bold: true
            color: textColour1
            smooth: true
        }
        Rectangle {
            id: prefsSeparator
            height: 1
            anchors.verticalCenter: prefsText.verticalCenter
            anchors.left: prefsText.right
            anchors.leftMargin: 7
            anchors.right: parent.right
            anchors.rightMargin: 10
            color: textColour1
            opacity: 0.5
            smooth: true
        }
        CheckBox {
            id: sortByNameCheckBox
            property int index: prefsText.index + 1
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: sortByName
            text: qsTr("sort by name?")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            smooth: true
            onEntered: {
                debug && console.log("[sortByNameCheckbox] focus: '" + focus + ", activeFocus: '" + activeFocus + "'");
            }
            onCheckedChanged: {
                 sortByName = checked;
                 var desc = gameListModel[gameListView.currentIndex].description
                 viewer.saveSettings();
                 viewer.loadGamelist();
                 gameListView.currentIndex = viewer.findIndex(desc, gameListView.currentIndex);
                 gameListView.positionViewAtIndex(lastIndex, ListView.Center);
                 debug && console.log("[sortByName] desc: '" + desc + "', " +
                                      "result: '" + viewer.findIndex(desc, gameListView.currentIndex) + "'");
            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: backendParamValuesCycleItem
            KeyNavigation.tab: autoHideToolbarCheckBox
        }

        CheckBox {
            id: autoHideToolbarCheckBox
            property int index: prefsText.index + 2
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: toolbarAutoHide
            text: qsTr("auto-hide toolbar")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            smooth: true
            onEntered: {
                debug && console.log("[autoHideToolbarCheckBox entered] focus: '" + focus + ", activeFocus: '" + activeFocus + "'");
            }
            onCheckedChanged: { toolbarAutoHide = checked; }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: sortByNameCheckBox
            KeyNavigation.tab: fpsCheckBox
        }
        CheckBox {
            id: fpsCheckBox
            property int index: prefsText.index + 3
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: fpsVisible
            text: qsTr("FPS counter")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            smooth: true
            onEntered: {
                debug && console.log("[fpsCheckBox entered] focus: '" + focus + ", activeFocus: '" + activeFocus + "'");
            }
            onCheckedChanged: {
                fpsVisible = checked;
                resetIgnoreLaunchTimer.restart();
                toolbarShowFpsLock = checked ? toolbarAutoHide : false;
            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: autoHideToolbarCheckBox
            KeyNavigation.tab: lightOutInputItem
        }
        InputItem {
            id: lightOutInputItem
            property int index: prefsText.index + 4
            height: parent.itemHeight
            inputWidth: 25
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            textPrefix: qsTr("lights out in")
            textSuffix: qsTr("secs")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            inputColour: "black"
            text: lightTimeout
            onAccepted: {
                text = text.replace(/([^0-9.])/g, '');
                var valid = text.match(/^(|\d+|\d+\.*\d+)$/g) &&
                              parseFloat(text) >= 5 ? true : false
                if (valid) {
                    inputColour = "black"
                    lightTimeout = parseFloat(text);
                    darkone.keepLightOn = lightTimeout == 0 ? true : false;
                    if (darkone.keepLightOn) {
                        lightOutTimer.stop();
                        lightOutScreenTimer.stop();
                    } else {
                        lightOutTimer.start();
                        lightOutScreenTimer.start();
                    }
                    focus = false;
                    overlayScaleSliderItem.focus = true;
                } else
                    inputColour = "red"
            }
            onFocusChanged: { if (focus)
                                  text = text.replace(/([^0-9.])/g, ''); }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: fpsCheckBox
            KeyNavigation.tab: overlayScaleSliderItem
        }
        SliderItem {
            id: overlayScaleSliderItem
            property int index: prefsText.index + 5
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            fgColour1: colour3
            fgColour2: colour4
            bgColour1: "white"
            bgColour2: "white"
            textPrefix: qsTr("scale")
            textSuffix: DarkoneJS.round(100 * overlayScale / DarkoneJS.overlayScaleMax, 0) + "%"
            sliderWidth: 85
            slidePercentage: 4
            onValueChanged: overlayScale = DarkoneJS.round(value, 2);
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: lightOutInputItem
            KeyNavigation.tab: backLightCheckBox
        }

        /********
        * effects
        */
        Text {
            id: prefsEffectsText
            property int index: 8
            opacity: 1.0
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            text: qsTr("effects")
            font.pixelSize: parent.itemTextSize + 2
            font.bold: true
            color: textColour1
            smooth: true
        }
        Rectangle {
            id: prefsEffectsSeparator
            height: 1
            anchors.verticalCenter: prefsEffectsText.verticalCenter
            anchors.left: prefsEffectsText.right
            anchors.leftMargin: 7
            anchors.right: parent.right
            anchors.rightMargin: 10
            color: textColour1
            opacity: 0.5
            smooth: true
        }
        CheckBox {
            id: backLightCheckBox
            property int index: prefsEffectsText.index + 1
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: backLight
            text: qsTr("back lighting")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            smooth: true
            onCheckedChanged: {
                backLightOpacity = checked ? darkone.opacity : 0;
                backLight = checked;
            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: overlayScaleSliderItem
            KeyNavigation.tab: launchFlashCheckBox
        }
        CheckBox {
            id: launchFlashCheckBox
            property int index: prefsEffectsText.index + 2
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: launchFlash
            text: qsTr("launch flash?")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            smooth: true
            onCheckedChanged: { launchFlash = checked; }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: backLightCheckBox
            KeyNavigation.tab: launchZoomCheckBox
        }
        CheckBox {
            id: launchZoomCheckBox
            property int index: prefsEffectsText.index + 3
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: launchZoom
            text: qsTr("launch zoom?")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            smooth: true
            onCheckedChanged: { launchZoom = checked; }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: launchFlashCheckBox
            KeyNavigation.tab: colourScheme1Button
        }

        /***************
        * colour schemes
        */
        Text {
            id: prefsColourSchemeText
            property int index: 12
            opacity: 1.0
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            text: qsTr("colour scheme")
            font.pixelSize: parent.itemTextSize + 2
            font.bold: true
            color: textColour1
            smooth: true
        }
        Rectangle {
            id: prefsColourSchemeSeparator
            height: 1
            anchors.verticalCenter: prefsColourSchemeText.verticalCenter
            anchors.left: prefsColourSchemeText.right
            anchors.leftMargin: 7
            anchors.right: parent.right
            anchors.rightMargin: 10
            color: textColour1
            opacity: 0.5
            smooth: true
        }
        ExclusiveGroup { id: checkGroup }
        CheckItem {
            id: colourScheme1Button
            property int index: prefsColourSchemeText.index + 1
            exclusiveGroup: checkGroup
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing) + 2
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: parent.itemHeight - 2
            text: qsTr("dark")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            onCheckedChanged: { 
                if ( checked ) { darkone.colourScheme = text; }
                debug && console.log("[colourScheme1Button] checked: '" + checked + "'");
            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: launchZoomCheckBox
            KeyNavigation.tab: colourScheme2Button
        }
        CheckItem {
            id: colourScheme2Button
            property int index: prefsColourSchemeText.index + 2
            exclusiveGroup: checkGroup
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing) - 2
            anchors.left: parent.left
            anchors.leftMargin: 10
            height: parent.itemHeight - 2
            text: qsTr("metal")
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            onCheckedChanged: {
                if ( checked ) { darkone.colourScheme = text; }
                debug && console.log("[colourScheme2Button] checked: '" + checked + "'");
            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: colourScheme1Button
            KeyNavigation.tab: backendParamNamesCycleItem
        }

        /**********
        * backend
        */
        Text {
            id: prefsBackendText
            property int index: 15
            opacity: 1.0
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            text: qsTr("backend")
            font.pixelSize: parent.itemTextSize + 2
            font.bold: true
            color: textColour1
            smooth: true
        }
        Rectangle {
            id: prefsBackendSeparator
            height: 1
            anchors.verticalCenter: prefsBackendText.verticalCenter
            anchors.left: prefsBackendText.right
            anchors.leftMargin: 7
            anchors.right: parent.right
            anchors.rightMargin: 10
            color: textColour1
            opacity: 0.5
            smooth: true
        }
        CycleItem {
            id: backendParamNamesCycleItem
            property int index: prefsBackendText.index + 1
            height: parent.itemHeight
            width: parent.width - 25
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            textPrefix: "param:"
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            items: viewer.cliParamNames()
            image: "../images/arrow.png"
            imageWidth: 14
            imageRotation: 0
            Component.onCompleted: {
                clicked.connect(backendParamValuesCycleItem.clicked);
            }
            onValueChanged: {
                debug && console.log("[preferences params names 1] param " +
                                         "values: '" + viewer.cliParamAllowedValues(backendParamNamesCycleItem.value) + "', " +
                                         "values set: '" + backendParamValuesCycleItem.items + "', " +
                                         "default: '" + viewer.cliParamValue(backendParamNamesCycleItem.value) + "', " +
                                         "default set: '" + backendParamValuesCycleItem.selectedItem + "'");

                backendParamValuesCycleItem.items = viewer.cliParamAllowedValues(value)
                backendParamValuesCycleItem.selectedItem = viewer.cliParamValue(backendParamNamesCycleItem.value)
                backendParamValuesCycleItem.value = viewer.cliParamValue(backendParamNamesCycleItem.value)

                debug && console.log("[preferences param names 2] param " +
                                         "values: '" + viewer.cliParamAllowedValues(backendParamNamesCycleItem.value) + "', " +
                                         "values set: '" + backendParamValuesCycleItem.items + "', " +
                                         "default: '" + viewer.cliParamValue(backendParamNamesCycleItem.value) + "', " +
                                         "default set: '" + backendParamValuesCycleItem.selectedItem + "'");

            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: colourScheme2Button
            KeyNavigation.tab: backendParamValuesCycleItem
        }
        Text {
            id: backendParamDescText
            property int index: prefsBackendText.index + 2
            opacity: 0.8
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            text: "desc:   "
            font.pixelSize: parent.itemTextSize
            font.bold: false
            color: textColour1
            smooth: true
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: { parent.opacity += 0.2; }
                onExited: { parent.opacity -= 0.2; }
            }
        }
        Text {
            id: backendParamDescText2
            opacity: 1.0
            anchors.verticalCenter: backendParamDescText.verticalCenter
            anchors.left: backendParamDescText.right
            anchors.leftMargin: 5
            anchors.right: parent.right
            anchors.rightMargin: 10
            text: viewer.cliParamDescription(backendParamNamesCycleItem.value)
            font.pixelSize: parent.itemTextSize + 1
            color: textColour1
            elide: Text.ElideRight
            verticalAlignment: Text.AlignVCenter
            horizontalAlignment: Text.AlignHCenter
            smooth: true
        }
        CycleItem {
            id: backendParamValuesCycleItem
            property int index: prefsBackendText.index + 3
            height: parent.itemHeight
            width: parent.width - 25
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            textSize: parent.itemTextSize
            textColour: textColour1
            activeColour: textColour2
            textPrefix: "value: "
            image: "../images/arrow.png"
            imageWidth: 14
            imageRotation: 0
            onClicked: {
                debug && console.log("[preferences] param values: '" +
                                          viewer.cliParamAllowedValues(backendParamNamesCycleItem.value) + "', " +
                                         "default: '" + viewer.cliParamValue(backendParamNamesCycleItem.value) + "'");
                if (value != "" && selectedItem != value) {
                    debug && console.log("[preferences] setting param: '" + backendParamNamesCycleItem.value + ", " +
                                                               "value: '" + selectedItem + "' -> '" + value + "'");
                    selectedItem = value;
                    viewer.setCliParamValue(backendParamNamesCycleItem.value, value);
                }

            }
            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: colourScheme2Button
            KeyNavigation.tab: sortByNameCheckBox
        }
    } // focusScope
    }


/***
* toolbar
*/
    Rectangle {
        id: toolbar
        x: 0
        z: 4
        width: DarkoneJS.baseWidth * DarkoneJS.scaleFactorX()
        height: 36
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        opacity: 0.75
        smooth: true
        state: "shown"
        gradient: Gradient {
            GradientStop { position: 0.0; color: colour1 }
            GradientStop { position: 0.2; color: colour2 }
            GradientStop { position: 0.7; color: colour3 }
            GradientStop { position: 1.0; color: colour4 }
        }
        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: { toolbarHidden && !DarkoneJS.inGame && DarkoneJS.toolbarToggle(); hideToolbarTimer.stop(); }
            onExited: { hideToolbarTimer.start(); }
            onPositionChanged: { if (!keepLightOn) {
                                     lightOutTimer.restart();
                                     lightOutScreenTimer.restart();
                                 }
                                 if (lightOut)
                                     DarkoneJS.lightToggle(1);
            }
        }
        transitions: [
            Transition {
                from: "hidden"
                to: "shown"
                ParallelAnimation {
                    PropertyAnimation { target: toolbar; property: "anchors.bottomMargin"; from: -(toolbar.height - 2); to: 0; duration: 500; easing.type: Easing.OutCubic }
                    PropertyAnimation { target: gameListView; property: "anchors.bottomMargin"; from: 2 + gameListView.itemHeight; to: toolbar.height + gameListView.itemHeight; duration: 500; easing.type: Easing.OutCubic }
                }
            },
            Transition {
                from: "shown"
                to: "hidden"
                ParallelAnimation {
                    PropertyAnimation { target: toolbar; property: "anchors.bottomMargin"; from: 0; to: -(toolbar.height - 2); duration: 500; easing.type: Easing.OutCubic }
                    PropertyAnimation { target: gameListView; property: "anchors.bottomMargin"; from: toolbar.height + gameListView.itemHeight; to: 2 + gameListView.itemHeight; duration: 500; easing.type: Easing.OutCubic }
                }
            }
        ]
        Item {
            id: searchBox
            width: DarkoneJS.listWidth()
            height: 24
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: parent.left
            anchors.leftMargin: 15
            opacity: 1.0
            Image {
                id: searchButton
                source: "images/find.png"
                height: 16
                anchors.right: searchTextInputBox.left
                anchors.rightMargin: 5
                anchors.verticalCenter: parent.verticalCenter
                fillMode: Image.PreserveAspectFit
                smooth: true
                opacity: 0.75
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: { toolbarAutoHide && toolbarHidden && !DarkoneJS.inGame && DarkoneJS.toolbarToggle(); hideToolbarTimer.stop(); parent.opacity = 1.0 }
                    onExited: { hideToolbarTimer.start(); parent.opacity = 0.75 }
                    onClicked: {
                        parent.opacity = 1.0;
                        gameListView.currentIndex = viewer.findIndex(searchTextInput.text, gameListView.currentIndex)
                        gameListView.positionViewAtIndex(gameListView.currentIndex, ListView.Center);
                        searchTextInput.focus = false;
                    }
                }
            }
            Rectangle {
                id: searchTextInputBox
                height: 18
                width: DarkoneJS.listWidth() - searchButton.width - 5 - clearButton.width - 5
                anchors.horizontalCenter: parent.horizontalCenter
                anchors.verticalCenter: parent.verticalCenter
                radius: height - 2
                smooth: true
                TextInput {
                    id: searchTextInput
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.left: parent.left
                    anchors.topMargin: 2
                    anchors.bottomMargin: 2
                    anchors.leftMargin: 8
                    anchors.right: parent.right
                    anchors.rightMargin: 8
                    font.pointSize: parent.height - 6
                    smooth: true
                    focus: false
                    cursorDelegate: Rectangle {
                        id: searchTextCursorDelegate
                        color: "black"
                        width: 1
                        height: 0.5
                        anchors.verticalCenter: parent.verticalCenter
                        visible: parent.activeFocus
                    }
                    onAccepted: { gameListView.currentIndex = viewer.findIndex(searchTextInput.text, gameListView.currentIndex)
                                  gameListView.positionViewAtIndex(gameListView.currentIndex, ListView.Center);
                    }
                    onFocusChanged: darkone.focus = !focus;
                }
            }
            Image {
                id: clearButton
                source: "images/clear.png"
                height: 18
                anchors.verticalCenter: parent.verticalCenter
                anchors.left: searchTextInputBox.right
                anchors.leftMargin: 5
                fillMode: Image.PreserveAspectFit
                smooth: true
                opacity: 0.75
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: parent.opacity = 1.0
                    onExited: parent.opacity = 0.75
                    onClicked: {
                        parent.opacity = 1.0;
                        searchTextInput.text = "";
                        searchTextInput.focus = false;
                    }
                }
            }
        }
        Image {
            id: showListButton
            source: "images/list_toggle.png"
            height: 18
            anchors.bottom: parent.bottom
            anchors.bottomMargin: (toolbar.height - height) / 2
            anchors.left: searchBox.right
            anchors.leftMargin: 15
            fillMode: Image.PreserveAspectFit
            opacity: 0.75
            rotation: listHidden ? 90 : 270
            smooth: true
            z: 5
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 1.0
                onExited: parent.opacity = 0.75
                onClicked: { DarkoneJS.listToggle(); }
            }
        }
        Image {
            id: preferencesButton
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: showListButton.right
            anchors.leftMargin: 10
            source: "images/preferences.png"
            smooth: true
            opacity: 0.75
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 1.0
                onExited: parent.opacity = 0.6
                onClicked: {
                    debug && console.log("[preferencesButton clicked] state: '" + preferencesDialog.state + "'")
                    debug && DarkoneJS.info("[preferencesButton clicked]", preferencesDialog)
                    parent.opacity = 1.0;
                    if (preferencesDialog.state == "shown") {
                        preferencesDialog.state = "hidden";
                        searchTextInput.focus = true;
                    } else {
                        preferencesDialog.state = "shown";
                        searchTextInput.focus = false;
                    }
                }
            }
        }
        Image {
            id: fullScreenToggleButton
            height: 16
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: preferencesButton.right
            anchors.leftMargin: 10
            source: "images/fullscreen.png"
            state: darkone.fullScreen ? "fullscreen" : "windowed"
            smooth: true
            opacity: 0.75
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 1.0
                onExited: parent.opacity = 0.6
                onClicked: {
                    if ( fullScreenToggleButton.state == "windowed" ) {
                        fullScreenToggleButton.state = "fullscreen"
                        darkone.fullScreen = true;
                    } else {
                        fullScreenToggleButton.state = "windowed"
                        darkone.fullScreen = false;
                    }
                    parent.opacity = 1.0;
                    searchTextInput.focus = false;
                }
            }
            states: [
                State {
                    name: "fullscreen"
                    PropertyChanges { target: fullScreenToggleButton; source: "images/windowed.png" }
                },
                State {
                    name: "windowed"
                    PropertyChanges { target: fullScreenToggleButton; source: "images/fullscreen.png" }
                }
            ]
        }
        Text {
            id: fpsText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: fullScreenToggleButton.right
            anchors.leftMargin: 15
            color: textColour1
            text: qsTr("FPS") + ": " + darkone.fps.toString()
            visible: darkone.fpsVisible
        }
        Rectangle {
            id: launchBox
            height: 20
            width: 40
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: overlay.anchors.rightMargin + overlay.width - overlayButtonBlock.x - (overlayButtonBlock.width / 2) - (width / 2);
            opacity: 0.5
            gradient: Gradient {
                GradientStop { position: 0.0; color: "transparent" }
                GradientStop { position: 0.25; color: DarkoneJS.gameStatusColour() }
                GradientStop { position: 0.75; color: DarkoneJS.gameStatusColour() }
                GradientStop { position: 1.0; color: "transparent" }
            }
            Image {
                id: launchButton
                source: "images/launch.png"
                height: 16
                width: 40
                anchors.verticalCenter: parent.verticalCenter
                anchors.horizontalCenter: parent.horizontalCenter
                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.CrossCursor
                    hoverEnabled: true
                    onEntered: { parent.opacity = 1.0
                                 parent.parent.opacity = 1.0
                                 overlayStateBlock.opacity = (lightOnAnimation.running || lightOut) ? 0 : 1.0;
                    }
                    onExited: { parent.opacity = 0.75
                                parent.parent.opacity = 0.5
                                overlayStateBlock.opacity = (lightOnAnimation.running || lightOut) ? 0 : 0.5;
                    }
                    onClicked: { if (!ignoreLaunch) {
                                     gameListView.positionViewAtIndex(lastIndex, ListView.Center);
                                     DarkoneJS.launch(); }
                    }
                }
            }
        }
        Image {
            id: exitButton
            height: 18
            anchors.verticalCenter: parent.verticalCenter
            anchors.right: parent.right
            anchors.rightMargin: 15
            source: "images/exit.png"
            smooth: true
            opacity: 0.25
            fillMode: Image.PreserveAspectFit
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: parent.opacity = 1.0
                onExited: parent.opacity = 0.6
                onClicked: {
                    parent.opacity = 1.0;
                    darkone.ignoreLaunch = true;
                    Qt.quit();
                }
            }
        }
    }

    focus: true
    Keys.onPressed: {
        switch ( event.key ) {
            case Qt.Key_Left: {
                if (!listHidden)
                    DarkoneJS.listToggle();
                event.accepted = true;
                break;
            }
            case Qt.Key_Right: {
                if (listHidden && preferencesDialog.state == "hidden")
                    DarkoneJS.listToggle();
                event.accepted = true;
                break;
            }
            case Qt.Key_Escape: {
                if ( searchTextInput.focus )
                    searchTextInput.focus = false;
                else if ( preferencesDialog.state == "shown" )
                    preferencesDialog.state = "hidden";
                else if ( launchFlashTimer.running ) {
                    launchFlashTimer.stop();
                    DarkoneJS.flashCounter = 0;
                    DarkoneJS.inGame = true; // fake game over
                    DarkoneJS.gameOver();
                }
                event.accepted = true;
                break;
            }
            case Qt.Key_F1:
                break;
            case Qt.Key_F11: {
                fullScreen = !fullScreen;
                event.accepted = true;
                break;
            }
            case Qt.Key_F:
            case Qt.Key_Enter:
            case Qt.Key_Return: {
                if ( event.modifiers & Qt.AltModifier ) {
                    fullScreen = !fullScreen;
                    event.accepted = true;
                }
                break;
            }
            case Qt.Key_Plus: {
                DarkoneJS.zoom(1.1);
                event.accepted = true;
                break;
            }
            case Qt.Key_Minus: {
                DarkoneJS.zoom(0.9);
                event.accepted = true;
                break;
            }
            case Qt.Key_D: {
                debug = !debug;
                event.accepted = true;
                break;
            }
            default: {
                if ( event.modifiers & Qt.ControlModifier) {
                    if ( event.modifiers & Qt.ShiftModifier ) {
                        switch ( event.key ) {
                            case Qt.Key_Up: {
                                DarkoneJS.zoom(1.1);
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_Down: {
                                DarkoneJS.zoom(0.9);
                                event.accepted = true;
                                break;
                            }
                        }
                    } else {
                        switch ( event.key ) {
                            case Qt.Key_L: {
                                if (preferencesDialog.state == "hidden")
                                    DarkoneJS.listToggle();
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_Q: {
                                Qt.quit();
                                break;
                            }
                            case Qt.Key_P: {
                                !darkone.ignoreLaunch && DarkoneJS.launch();
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_O: {
                                preferencesDialog.state = preferencesDialog.state == "shown" ? "hidden" : "shown";
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_T: {
                                DarkoneJS.toolbarToggle();
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_X: {
                                if ( confirmQuitDialog.state == "hidden" )
                                    confirmQuitDialog.state = "shown";
                                else
                                    confirmQuitDialog.state = "hidden";
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_S: {
                                if ( !darkone.toolbarHidden ) {
                                    searchTextInput.text = "";
                                    searchTextInput.focus = true;
                                }
                                break;
                            }
                        }
                    }
                } else if ( !darkone.toolbarHidden ) {
                    if ( DarkoneJS.validateKey(event.text) ) {
                        searchTextInput.text += event.text;
                        searchTextInput.focus = true;
                    } else if ( DarkoneJS.validateSpecialKey(event.text) ) {
                        searchTextInput.focus = true;
                        switch ( event.text ) {
                        case "\b":
                            if ( searchTextInput.text.length > 0)
                                searchTextInput.text = searchTextInput.text.substring(0, searchTextInput.text.length - 1);
                            break;
                        }
                    }
                }
            }
        }
    }
    Keys.forwardTo: [gameListView]
//}

}
