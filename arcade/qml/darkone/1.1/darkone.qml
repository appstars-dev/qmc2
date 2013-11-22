import QtQuick 1.1;
import "./components";
import Wheel 1.0;
import Pointer 1.0;
import "darkone.js" as DarkoneJS;

FocusScope {
    id: darkoneFocusScope
    focus: true
 
    width: darkone.width
    height: darkone.height
  
    // global properties
    property alias fps: darkone.fps
    property alias debug: darkone.debug
    property alias debug2: darkone.debug2

    // restored properties
    property alias lastIndex: darkone.lastIndex
    property alias toolbarHidden: darkone.toolbarHidden
    property alias listHidden: darkone.listHidden
    property alias fullScreen: darkone.fullScreen
    property alias fpsVisible: darkone.fpsVisible
    property alias sortByName: darkone.sortByName
    property alias launchFlash: darkone.launchFlash
    property alias launchZoom: darkone.launchZoom
    property alias dataTypePrimary: darkone.dataTypePrimary
    property alias dataTypeSecondary: darkone.dataTypeSecondary
    property alias lightTimeout: darkone.lightTimeout
    property alias backLight: darkone.backLight
    property alias backLightOpacity: darkone.backLightOpacity
    property alias screenLight: darkone.screenLight
    property alias screenLightOpacity: darkone.screenLightOpacity
    property alias toolbarAutoHide: darkone.toolbarAutoHide
    property alias overlayScale: darkone.overlayScale
    property alias colourScheme: darkone.colourScheme
 
Rectangle {
    id: darkone
    focus: true
    z: 0
    anchors.fill: parent   
    width: DarkoneJS.baseWidth
    height: DarkoneJS.baseHeight

    // global properties
    property bool debug: false
    property bool debug2: false
    property int fps: 0

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
    property bool screenLight: true
    property real screenLightOpacity: 0
    property bool backLight: true
    property real backLightOpacity: 0
    property bool toolbarAutoHide: true
    property real overlayScale: 1
    property string colourScheme: "dark"

    property bool initialised: false
    property bool ignoreLaunch: false
    property bool dataHidden: true
    property bool keepLightOn: false
    property bool lightOut: true
    property bool lightOutScreen: true
    property string dataTypeCurrent: "title"
    property int zoomDuration: 250
    property int listDuration: 750
    property int overlayDuration: 0
    property int flashes: 4
    property int toolbarHideIn: 0
    property bool preferencesLaunchLock: false
    property bool toolbarShowMenuLock: false
    property bool toolbarShowFpsLock: false
    property bool infoMissing: true
    property string colour1: "#000000"
    property string colour2: "#000000"
    property string colour3: "#000000"
    property string colour4: "#000000"
    property string colour5: "#000000"
    property string textColour1: "#000000"
    property string textColour2: "#000000"
    property string version: ""

    color: "black"
    opacity: 0
    state: "off"

    Component.onCompleted: { initTimer.start(); }
    Connections {
        target: viewer;
        onEmulatorStarted: DarkoneJS.gameOn();
        onEmulatorFinished: DarkoneJS.gameOver();
    }

    onToolbarAutoHideChanged: { debug && console.log("toolbarAutoHide: '" + darkone.toolbarAutoHide + "'"); }
    onLastIndexChanged: { debug && console.log("lastIndex: '" + darkone.lastIndex + "'"); }
    onColourSchemeChanged: { DarkoneJS.colourScheme(colourScheme); }
    onStateChanged: state == "off" ? lightOffAnimation.start() : lightOnAnimation.start()
    onLightOutChanged: { debug && console.log("[darkone] darkone.lightOut: '" + darkone.lightOut + ", " +
                                                        "state before: '" + darkone.state + "'");
                         darkone.lightOut ? darkone.state = "off" : darkone.state = "on"; }
    onLightOutScreenChanged: { debug && console.log("[darkone] darkone.lightOutScreen: '" + darkone.lightOutScreen + ", " +
                                                              "overlayScreen.state orig: '" + overlayScreen.state + "', " +
                                                              "screenLightOpacity: '" + darkone.screenLightOpacity + "'");
                              if (darkone.lightOutScreen) {
                                  overlayScreenLight.visible = false;
                                  overlayBackLight.visible = false;
                                  overlayScreen.state = "off"
                               } else {
                                  if (screenLight)
                                     overlayScreenLight.visible = true;
                                  if (backLight)
                                     overlayBackLight.visible = true;
                                  overlayScreen.state = "on";
                               } }
    onDataHiddenChanged: { darkone.dataHidden ? overlayData.state = "hidden" : overlayData.state = "shown"; }
    onInfoMissingChanged: { darkone.infoMissing ? overlayText.state = "missing" : overlayText.state = "found"; }
    onOverlayScaleChanged: { overlayScaleSliderItem.value = darkone.overlayScale; }
    onFullScreenChanged: {
        if ( !DarkoneJS.initialising ) {
            if ( darkone.fullScreen ) {
                viewer.switchToFullScreen();
                fullScreenToggleButton.state = "fullscreen";
            } else {
                viewer.switchToWindowed();
                fullScreenToggleButton.state = "windowed";
            }
        }
    }
    onFocusChanged: { debug2 && focus && DarkoneJS.inFocus(); }

    PropertyAnimation { id: fadeIn; target: darkone; property: "opacity"; duration: 2000; from: 0; to: 1.0; easing.type: Easing.InExpo; }
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

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        onPositionChanged: { if (!darkone.keepLightOn) {
                                 lightOutTimer.restart();
                                 lightOutScreenTimer.restart();
                             }
                             if (darkone.lightOut)
                                 DarkoneJS.lightToggle(1);
        }
    }
    // global key events
    Keys.onPressed: {
        debug2 && console.log("[keys] darkone: '" + DarkoneJS.keyEvent2String(event) + "'")
        switch ( event.key ) {
            case Qt.Key_Left: {
                if (!darkone.listHidden)
                    DarkoneJS.listToggle();
                event.accepted = true;
                break;
            }
            case Qt.Key_Right: {
                if (listHidden && preferences.state == "hidden")
                    DarkoneJS.listToggle();
                event.accepted = true;
                break;
            }
            case Qt.Key_Escape: {
                if ( searchTextInput.focus )
                    searchTextInput.focus = false;
                else if ( preferences.state == "shown" )
                    preferences.state = "hidden";
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
                darkone.fullScreen = !darkone.fullScreen;
                event.accepted = true;
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
            default: {
                if ( event.modifiers & Qt.AltModifier) {
                    switch ( event.key ) {
                        case Qt.Key_Enter:
                        case Qt.Key_Return: {
                            darkone.fullScreen = !darkone.fullScreen;
                            event.accepted = true;
                            break;
                        }
                    }
                } else if ( event.modifiers & Qt.ControlModifier) {
                    if ( event.modifiers & Qt.ShiftModifier ) {
                    } else {
                        switch ( event.key ) {
                            case Qt.Key_D: {
                                debug = !debug;
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_K: {
                                debug2 = !debug2;
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_L: {
                                if (preferences.state == "hidden")
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
                                preferences.state = preferences.state == "shown" ? "hidden" : "shown";
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_T: {
                                DarkoneJS.toolbarToggle();
                                event.accepted = true;
                                break;
                            }
                            case Qt.Key_S: {
                                if ( !darkone.toolbarHidden ) {
                                    searchTextInput.text = "";
                                    searchTextInput.focus = true;
                                    searchTextInput.forceActiveFocus();
                                }
                                break;
                            }
                        }
                    }
                }
            }
        }
    }
    Timer {
        id: initTimer
        interval: 5
        running: false
        repeat: true
        onTriggered: { DarkoneJS.init(); }
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
            if (darkone.toolbarHideIn == 0) {
                hideToolbarTimer.stop();
                darkone.toolbarHideIn = 3;
                darkone.toolbarAutoHide && DarkoneJS.toolbarToggle(-1);
            } else
                darkone.toolbarHideIn -= 1;
        }
    }
    Timer {
        id: lightOutTimer
        interval: darkone.lightTimeout * 1000
        running: false
        repeat: true
        onTriggered: DarkoneJS.lightToggle(-1);
    }
    Timer {
        id: lightOutScreenTimer
        interval: darkone.lightTimeout * 1000 + 2500
        running: false
        repeat: true
        onTriggered: { darkone.lightOutScreen = true;
                       lightOutScreenTimer.stop(); }
    }


/***
* overlay
*/
    Rectangle {
        id: overlay
        focus: true // darkoneFocusScope
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

        Behavior on width { PropertyAnimation { duration: darkone.overlayDuration; easing.type: Easing.InOutQuad } }

        onFocusChanged: {
            if ( focus ) {
                overlayScreen.focus = true;
                if ( darkone.initialised )
                    debug2 && focus && DarkoneJS.inFocus();
            }
        }
        MouseArea {
            anchors.fill: parent
            onClicked: {
                debug && console.log("[overlay] clicked");
                overlay.focus = true;
            }
        }
        WheelArea {
            anchors.fill: parent
            onWheel: {
                       DarkoneJS.zoom(1 + (0.1) * (delta / Math.abs(delta)));
                       debug && console.log("[overlay] wheel event: darkone.overlayScale: '" + darkone.overlayScale + "', " +
                                                                   "zoom: '" + (1 + (0.1) * (delta / Math.abs(delta))) + "'");
            }
        }
        // overlay key events
        Keys.onPressed: {
            debug2 && console.log("[keys] overlay: '" + DarkoneJS.keyEvent2String(event) + "'")
            switch( event.key ) {
                case Qt.Key_1: {
                    if ( ! darkone.dataHidden )
                        darkone.dataTypePrimary = darkone.dataTypeCurrent
                    event.accepted = true;
                    break;
                }
                case Qt.Key_2: {
                    if ( ! darkone.dataHidden )
                        darkone.dataTypeSecondary = darkone.dataTypeCurrent
                    event.accepted = true;
                    break;
                }
                case Qt.Key_F: {
                    if ( event.modifiers & Qt.AltModifier ) {
                        darkone.fullScreen = !darkone.fullScreen;
                        event.accepted = true;
                    }
                    break;
                }
                case Qt.Key_Enter:
                case Qt.Key_Return: {
                    darkone.dataHidden = !darkone.dataHidden
                    if ( ! darkone.dataHidden )
                       overlayDataTypeCycleItem.focus = true;
                    else
                       overlay.focus = true;
                    event.accepted = true;
                    break;
                }
                case Qt.Key_D: {
                    debug = !debug;
                    event.accepted = true;
                    break;
                }
                case Qt.Key_K: {
                    debug2 = !debug2;
                    event.accepted = true;
                    break;
                }
                case Qt.Key_P: {
                    !darkone.ignoreLaunch && DarkoneJS.launch();
                    event.accepted = true;
                    break;
                }
                case Qt.Key_Tab: {
                    if ( event.modifiers & Qt.ShiftModifier ) {
                        if ( !darkone.toolbarHidden )
                            toolbarFocusScope.focus = true;
                        else if ( !darkone.listHidden )
                            gameListView.focus = true;
                    } else {
                        if ( !darkone.listHidden )
                            gameListView.focus = true;
                        else if ( !darkone.toolbarHidden )
                            toolbarFocusScope.focus = true;
                    }
                    event.accepted = true;
                    break;
                }
                case Qt.Key_Up: {
                    if ( event.modifiers & Qt.ControlModifier) {
                        if ( event.modifiers & Qt.ShiftModifier ) {
                            DarkoneJS.zoom(1.1);
                            event.accepted = true;
                            break;                        
                        } else {
                            // scroll page
                            if ( overlayText.text != "" ) {
                                overlayTextFlick.contentY += overlayTextFlick.height / 10 * 9
                                overlayTextFlick.returnToBounds();
                            }
                            event.accepted = true;
                            break;
                        }
                    } else {
                        // scroll line
                        if ( overlayText.text != "" ) {
                            overlayTextFlick.contentY += overlayTextFlick.height / 10
                            overlayTextFlick.returnToBounds();
                        }
                        event.accepted = true;
                        break;
                    }
                }
                case Qt.Key_Down: {
                    if ( event.modifiers & Qt.ControlModifier) {
                        if ( event.modifiers & Qt.ShiftModifier ) {
                            DarkoneJS.zoom(0.9);
                            event.accepted = true;
                            break;                        
                        } else {
                            // scroll page
                            if ( overlayText.text != "" ) {
                                overlayTextFlick.contentY -= overlayTextFlick.height / 10 * 9
                                overlayTextFlick.returnToBounds();
                            }
                            event.accepted = true;
                            break;
                        }
                    } else {
                        // scroll line
                        if ( overlayText.text != "" ) {
                            overlayTextFlick.contentY -= overlayTextFlick.height / 10
                            overlayTextFlick.returnToBounds();
                        }
                        event.accepted = true;
                        break;
                    }
                }
                case Qt.Key_Left:
                case Qt.Key_Right: {
                    if ( ! darkone.dataHidden )
                        event.accepted = true
                    break;
                }
                default: {
                }
            }
        }

/***
* screen
*/
        Rectangle {
            id: overlayScreen
            z: 1
            width: 354
            height: 262 - 10
            scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
            anchors.top: parent.top
            // keep the screen still under scaling, ensure margin of 30% of non-screen space
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (height * scale))
            anchors.horizontalCenter: parent.horizontalCenter
            smooth: true
            opacity: 1.0
            border.color: debug ? "blue" : "black"
            border.width: debug ? 2 : 1
            color: debug ? "white" : "#181818"
            state: "on"
            focus: true

            Behavior on scale { PropertyAnimation { id: "overlayScreenScaleAnimation"; properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
            transitions: [
                Transition {
                    from: "on"
                    to: "off"
                    ParallelAnimation {
                        PropertyAnimation { target: overlayScreen; property: "opacity"; from: 1.0; to: 0.1; duration: 500; easing.type: Easing.OutExpo }
                        PropertyAnimation { target: overlayBackLight; property: "opacity"; from: darkone.screenLightOpacity; to: 0; duration: 500; easing.type: Easing.OutExpo }

                    }
                },
                Transition {
                    from: "off"
                    to: "on"
                    ParallelAnimation {
                        PropertyAnimation { target: overlayScreen; property: "opacity"; from: 0.1; to: 1.0; duration: 500; easing.type: Easing.OutExpo }
                        PropertyAnimation { target: overlayBackLight; property: "opacity"; from: 0; to: darkone.screenLightOpacity; duration: 500; easing.type: Easing.OutExpo }
                    }
                }
            ]

            onActiveFocusChanged: {
                if ( darkone.initialised )
                    debug2 && focus && DarkoneJS.inFocus();
                    if ( !darkone.dataHidden )
                        overlayDataTypeCycleItem.focus = true;
            }
            onStateChanged: { debug && console.log("[overlayScreen] state changed, state: '" + state + "', " +
                                                                   "screenLightOpacity: '" + darkone.screenLightOpacity + "'"); }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onClicked: { 
                    parent.forceActiveFocus();
                }
                onDoubleClicked: { debug && console.log("[overlayScreen] double-clicked");
                                   darkone.dataHidden = !darkone.dataHidden; }
                onEntered: { debug && console.log("[overlayScreen] entered");
                             debug2 && console.log(overlayDataTypeCycleItem.focus + "|" + overlayDataTypeCycleItem.activeFocus); }
                onPositionChanged: { if (!darkone.keepLightOn) {
                                         lightOutTimer.restart();
                                         lightOutScreenTimer.restart();
                                     }
                                     if (darkone.lightOut)
                                         DarkoneJS.lightToggle(1);
                }
            }

            Rectangle {
                id: overlayScreenBorderTop
                z: parent.z + 15
                anchors.top: parent.top
                anchors.topMargin: -2
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: 3
                visible: parent.focus || overlayDataTypeCycleItem.focus
                color: darkone.textColour2
            }
            Rectangle {
                id: overlayScreenBorderBottom
                z: parent.z + 15
                anchors.bottom: parent.bottom
                anchors.bottomMargin: -2
                anchors.horizontalCenter: parent.horizontalCenter
                width: parent.width
                height: 3
                visible: parent.focus || overlayDataTypeCycleItem.focus
                color: darkone.textColour2
            }


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

                Behavior on anchors.topMargin { PropertyAnimation { duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                Behavior on anchors.bottomMargin { PropertyAnimation { duration: darkone.zoomDuration; easing.type: Easing.Linear } }
                CursorShapeArea {
                    anchors.fill: parent
                    cursorShape: Qt.CrossCursor
                }
                MouseArea {
                    anchors.fill: parent
                    onDoubleClicked: { debug && console.log("[overlayDisplay] double-clicked");
                                       darkone.dataHidden = !darkone.dataHidden; }
                }

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
                                           darkone.dataHidden = !darkone.dataHidden; }
                    }
                    WheelArea {
                        anchors.fill: parent
                        onWheel: {
                            DarkoneJS.zoom(1 + (0.1) * (delta / Math.abs(delta)));
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
                                               darkone.dataHidden = !darkone.dataHidden; }
                        }
                        WheelArea {
                            anchors.fill: parent
                            onWheel: {
                                DarkoneJS.zoom(1 + (0.1) * (delta / Math.abs(delta)));
                            }
                        }

                        Text {
                            id: overlayText
                            anchors.fill: parent
                            text: DarkoneJS.data("text")
                            textFormat: Text.RichText
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

                Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
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

                CursorShapeArea {
                    anchors.fill: parent
                    cursorShape: Qt.CrossCursor
                }

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
                        onClicked: { darkone.dataHidden = !darkone.dataHidden; }
                    }

                    Text {
                          id: overlayDataTitle
                          text: DarkoneJS.gameCardHeader()
                          textFormat: Text.RichText
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
                                           if (!darkone.dataHidden)
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

                        CursorShapeArea {
                            anchors.fill: parent
                            cursorShape: Qt.CrossCursor
                        }
                        MouseArea {
                            anchors.fill: parent
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

                        CursorShapeArea {
                            anchors.fill: parent
                            cursorShape: Qt.CrossCursor
                        }
                        MouseArea {
                            anchors.fill: parent
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
                    CycleItem {
                        id: overlayDataTypeCycleItem
                        property int index: prefsBackendText.index + 1
                        anchors.fill: parent
                        anchors.topMargin: 5 + 1
                        anchors.horizontalCenter: parent.horizontalCenter
                        anchors.verticalCenter: parent.verticalCenter
                        anchors.leftMargin: parent.width / 4
                        anchors.rightMargin: parent.width / 4
                        height: parent.itemHeight
                        width: parent.width / 2
                        textSize: 12
                        textColour: "white"
                        items: DarkoneJS.datatypeKeys();
                        image: "../images/arrow.png"
                        imageWidth: 16
                        imageRotation: 0
                        activeColour: "transparent"
                        passKeyEvents: true

                        Component.onCompleted: {
                            value = darkone.dataTypeCurrent;
                            text = DarkoneJS.data("name");
                        }

                        onValueChanged: {
                            darkone.dataTypeCurrent = value;
                            text = DarkoneJS.data("name");
                        }
                        onFocusChanged: { debug2 && focus && DarkoneJS.inFocus(); }

                        Keys.onPressed: {
                            debug2 && console.log("[keys] overlayDataTypeCycleItem: '" + DarkoneJS.keyEvent2String(event) + "'")
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
            scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
            anchors.top: parent.top
            //keep the cabinet still under scaling, then shift it by the same amount as the screen is shift, then offet the image to match the screen
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) - (558 * scale)
            anchors.horizontalCenter: overlayScreen.horizontalCenter
            anchors.horizontalCenterOffset: 0 - 1.5 * darkone.overlayScale // manual screen/cabinet alignment tweaks
            smooth: true
            opacity: 1.0

            Behavior on scale { ParallelAnimation {
                                  PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear }
                                  PropertyAnimation { target: overlayBackLight; properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } } }
        }
        Image {
            z: 5
            id: overlayScreenLight
            source: "images/screenlight.png"
            fillMode: Image.PreserveAspectFit
            scale: overlayCabinet.scale
            anchors.top: parent.top
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) - (558 * scale)
            anchors.horizontalCenter: overlayScreen.horizontalCenter
            anchors.horizontalCenterOffset: overlayCabinet.anchors.horizontalCenterOffset
            smooth: true
            opacity: darkone.screenLight ? darkone.screenLightOpacity : 0
        }
        Image {
            z: 0
            id: overlayBackLight
            source: "images/backlight.png"
            fillMode: Image.PreserveAspectFit
            scale: overlayCabinet.scale
            anchors.top: parent.top
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) - (558 * scale)
            anchors.horizontalCenter: overlayScreen.horizontalCenter
            anchors.horizontalCenterOffset: overlayCabinet.anchors.horizontalCenterOffset
            smooth: true
            opacity: darkone.backLight ? darkone.backLightOpacity : 0
        }
        Rectangle {
            id: overlayStateBlock
            z: 0
            width: 150
            height: 275
            scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
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
            scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
            anchors.top: parent.top
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) + (319 * scale)
            anchors.horizontalCenter: overlayCabinet.horizontalCenter
            anchors.horizontalCenterOffset: 2 * darkone.overlayScale
            color: debug ? "white" : "transparent"

            Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }

            CursorShapeArea {
                anchors.fill: parent
                cursorShape: Qt.CrossCursor
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: { overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 1.0; }
                onExited: { overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 0.5; }
                onClicked: { if (!darkone.ignoreLaunch) {
                                 gameListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                                 DarkoneJS.launch(); }
                }
            }
        }
        Rectangle {
            id: overlayGridBlock
            z: 2
            width: 120
            height: 40
            scale: DarkoneJS.scaleFactorY() * darkone.overlayScale
            anchors.top: parent.top
            anchors.topMargin: (((scale * height) - height) / 2) + 0.3 * (darkone.height - 20 - (overlayScreen.height * overlayScreen.scale)) + (500 * scale)
            anchors.horizontalCenter: overlayCabinet.horizontalCenter
            anchors.horizontalCenterOffset: 1 * darkone.overlayScale
            color: debug ? "white" : "transparent"

            Behavior on scale { PropertyAnimation { properties: "scale"; duration: darkone.zoomDuration; easing.type: Easing.Linear } }
            CursorShapeArea {
                anchors.fill: parent
                cursorShape: Qt.CrossCursor
            }
            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
                onEntered: { overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 1.0; }
                onExited: { overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 0.5; }
                onClicked: { if (!darkone.ignoreLaunch) {
                                 gameListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                                 DarkoneJS.launch(); }
                }
            }
        }
    } // overlay



/***
* list
*/

    Rectangle {
        id: gameListViewBorder
        z: gameListView.z + 15
        anchors.left: darkone.left
        anchors.leftMargin: 8
        anchors.top: darkone.top
        anchors.topMargin: gameListView.itemHeight + 1
        width: 2
        height: gameListView.height - 2
        color: darkone.textColour2
        visible: gameListView.activeFocus ? true : false
    }

    ListView {
        id: gameListView
        focus: false // darkoneFocusScope
        property int itemHeight: 24
        z: 3
        height: parent.height - (darkone.toolbarHidden ? 2 : toolbar.height) - gameListView.itemHeight - gameListView.itemHeight
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
                        PropertyAnimation { target: gameListView; property: "anchors.leftMargin"; from: -DarkoneJS.listWidth() - 5; to: 15; duration: darkone.listDuration; easing.type: Easing.InOutQuad }

                        PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: -DarkoneJS.listWidth() - 5; to: 15; duration: darkone.listDuration; easing.type: Easing.InOutQuad } }
                    PropertyAnimation { target: gameListViewBorder; property: "opacity"; from: 0; to: 1.0; duration: 0; } } },
            Transition {
                from: "shown"
                to: "hidden"
                SequentialAnimation {
                    PropertyAnimation { target: gameListViewBorder; property: "opacity"; from: 1.0; to: 0; duration: 0; }
                    // ensure correct initial position
                    PropertyAnimation { target: gameListView; property: "anchors.leftMargin"; from: anchors.leftMargin; to: DarkoneJS.listWidth() +15; duration: 0; easing.type: Easing.Linear }
                    PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: anchors.leftMargin; to: DarkoneJS.listWidth() + 15; duration: 0; easing.type: Easing.Linear }
                    // animate
                    ParallelAnimation {
                        PropertyAnimation { target: gameListView; property: "anchors.leftMargin"; from: 15; to: -DarkoneJS.listWidth() - 5; duration: darkone.listDuration; easing.type: Easing.InOutQuad }
                        PropertyAnimation { target: searchBox; property: "anchors.leftMargin"; from: 15; to: -DarkoneJS.listWidth() - 5; duration: darkone.listDuration; easing.type: Easing.InOutQuad } }
                    // make invisible (set in the state property)
                    PropertyAnimation { target: gameListView; property: "opacity"; duration: 0; }
                    PropertyAnimation { target: searchBox; property: "opacity"; duration: 0; }
                    // re-anchor show/hide list button (set in the state property)
                    PropertyAnimation { target: showListButton; property: "anchors.left"; duration: 0; }
             } }
        ]

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

        onCurrentIndexChanged: { 
            if ( darkone.initialised )
                darkone.lastIndex = currentIndex;
        }

        CursorShapeArea {
            anchors.fill: parent
            cursorShape: Qt.ArrowCursor
        }
        // gameListView key events
        Keys.onPressed: {
            debug2 && console.log("[keys] gameListView: '" + DarkoneJS.keyEvent2String(event) + "'")
            if (!darkone.keepLightOn) {
                lightOutTimer.restart();
                lightOutScreenTimer.restart();
            }
            if (darkone.lightOut)
                DarkoneJS.lightToggle(1);
            switch ( event.key ) {
                case Qt.Key_Tab: {
                    if ( event.modifiers & Qt.ShiftModifier )
                        overlay.focus = true;
                    else {
                        if ( !darkone.toolbarHidden )
                            toolbarFocusScope.focus = true;
                        else
                            overlay.focus = true;
                    }
                    event.accepted = true;
                    break;
                }
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
                    if ( !(event.modifiers & Qt.AltModifier) && !darkone.ignoreLaunch ) {
                        gameListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
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

        // item
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
                              GradientStop { position: 0.0; color: darkone.colour1 }
                              GradientStop { position: 0.2; color: darkone.colour2 }
                              GradientStop { position: 0.7; color: darkone.colour3 }
                              GradientStop { position: 1.0; color: darkone.colour4 } }
                opacity: 0.75

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
                    onDoubleClicked: { if (!darkone.ignoreLaunch) {
                                          gameListView.currentIndex = index;
                                          gameListView.positionViewAtIndex(darkone.lastIndex, ListView.Center) ;
                                          DarkoneJS.launch(); }
                    }
                    onClicked: {
                        gameListView.currentIndex = index;
                        debug && console.log("[gameListView] setting index: '" + index + "'");
                        gameListView.focus = true;
                        gameListView.forceActiveFocus();
                    }
                }

                Text {
                    property bool fontResized: false
                    id: gameListItemText
                    anchors.fill: parent
                    anchors.margins: 10
                    verticalAlignment: Text.AlignVCenter
                    horizontalAlignment: Text.AlignHCenter
                    text: model.modelData.description
                    color: gameListItemBackground.ListView.isCurrentItem ? darkone.textColour2: darkone.textColour1
                    font.bold: true
                    font.pixelSize: 13
                    elide: Text.ElideRight
                    smooth: true
                }
            }
        }
    }


/***
* preferences menu
*/

    FocusScope {
    id: preferencesFocusScope
    focus: false // darkoneFocusScope

    Rectangle {
        id: preferences
        z: 4
        y: darkone.height - toolbar.height - 5 - height
        x: preferencesButton.x - 10
        property int itemHeight: 12
        property int itemSpacing: 6
        property int itemTextSize: 9
        property string activeColour: darkone.textColour2
        height: (itemHeight + itemSpacing) * 22 + 10
        width: 175
        smooth: true
        border.color: parent.focus ? activeColour : "transparent";
        border.width: parent.focus ? 1 : 0;
        color: darkone.colour5
        opacity: 1.0
        state: "hidden"

        states: [
            State {
                name: "hidden"
                PropertyChanges { target: preferences; visible: false; }
            },
            State {
                name: "shown"
                PropertyChanges { target: preferences; visible: true; }
            }
        ]
        transitions: Transition {
            from: "hidden"
            to: "shown"
            reversible: true
            PropertyAnimation { property: "opacity"; duration: 100 }
        }

        onStateChanged: {
            if ( state == "shown" ) {
                darkone.ignoreLaunch = true;
                darkone.preferencesLaunchLock = true;
                darkone.toolbarShowMenuLock = true;
                overlayScaleSliderItem.maximum = DarkoneJS.overlayScaleMax * 1.5;
                overlayScaleSliderItem.minimum = DarkoneJS.overlayScaleMin;
                overlayScaleSliderItem.value = darkone.overlayScale;
                preferencesFocusScope.focus = true;
            } else {
                darkone.preferencesLaunchLock = false;
                darkone.ignoreLaunch = false;
                darkone.toolbarShowMenuLock = false;
                overlay.focus = true;
            }
        }
        onFocusChanged: {
            debug2 && focus && DarkoneJS.inFocus();
        }

        CursorShapeArea {
            anchors.fill: parent
            cursorShape: Qt.ArrowCursor
        }
        MouseArea {
            anchors.fill: parent;
            hoverEnabled: true
            onClicked: {
                debug && console.log("[preferences onClick 1] focus: '" + focus + "'");
                preferencesFocusScope.focus = true;
                debug && console.log("[preferences onClick 2] focus: '" + focus + "'");
           }
        }
        // preferences key events
        Keys.onPressed: {
            debug2 && console.log("[keys] preferences: '" + DarkoneJS.keyEvent2String(event) + "'")
            switch( event.key ) {
                case Qt.Key_Tab: {
                    if ( event.modifiers & Qt.ShiftModifier ) {
                        backendParamValuesCycleItem.focus = true;
                        backendParamValuesCycleItem.forceActiveFocus()
                    } else {
                        sortByNameCheckBox.focus = true;
                        sortByNameCheckBox.forceActiveFocus();
                    }
                    event.accepted = true;
                    break;
                }
            }
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
            font.pixelSize: preferences.itemTextSize + 3
            font.bold: true
            color: darkone.textColour1
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
            font.pixelSize: preferences.itemTextSize + 2
            font.bold: true
            color: darkone.textColour1
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
            color: darkone.textColour1
            opacity: 0.5
            smooth: true
        }
        CheckBox {
            id: sortByNameCheckBox
            focus: true // preferencesFocusScope
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
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            smooth: true

            onEntered: {
                debug && console.log("[sortByNameCheckbox] focus: '" + focus + ", activeFocus: '" + activeFocus + "'");
            }
            onCheckedChanged: {
                if (darkone.initialised) {
                    sortByName = checked;
                    var desc = gameListModel[gameListView.currentIndex].description
                    viewer.saveSettings();
                    viewer.loadGamelist();
                    gameListView.currentIndex = viewer.findIndex(desc, gameListView.currentIndex);
                    gameListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
                    debug && console.log("[sortByName] desc: '" + desc + "', " +
                                         "result: '" + viewer.findIndex(desc, gameListView.currentIndex) + "'");
                }
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
            checked: darkone.toolbarAutoHide
            text: qsTr("auto-hide toolbar")
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            smooth: true

            onEntered: {
                debug && console.log("[autoHideToolbarCheckBox entered] focus: '" + focus + ", activeFocus: '" + activeFocus + "'");
            }
            onCheckedChanged: { darkone.toolbarAutoHide = checked; }

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
            checked: darkone.fpsVisible
            text: qsTr("FPS counter")
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            smooth: true

            onEntered: {
                debug && console.log("[fpsCheckBox entered] focus: '" + focus + ", activeFocus: '" + activeFocus + "'");
            }
            onCheckedChanged: {
                darkone.fpsVisible = checked;
                resetIgnoreLaunchTimer.restart();
                darkone.toolbarShowFpsLock = checked ? darkone.toolbarAutoHide : false;
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
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            inputColour: "black"
            text: darkone.lightTimeout

            onAccepted: {
                text = text.replace(/([^0-9.])/g, '');
                var valid = text.match(/^(|\d+|\d+\.*\d+)$/g) &&
                              parseFloat(text) >= 5 ? true : false
                if (valid) {
                    inputColour = "black"
                    darkone.lightTimeout = parseFloat(text);
                    darkone.keepLightOn = darkone.lightTimeout == 0 ? true : false;
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
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            fgColour1: darkone.colour3
            fgColour2: darkone.colour4
            bgColour1: "white"
            bgColour2: "white"
            textPrefix: qsTr("scale")
            textSuffix: DarkoneJS.round(100 * darkone.overlayScale / DarkoneJS.overlayScaleMax, 0) + "%"
            sliderWidth: 85
            slidePercentage: 4

            onValueChanged: darkone.overlayScale = DarkoneJS.round(value, 2);

            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: lightOutInputItem
            KeyNavigation.tab: screenLightCheckBox
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
            font.pixelSize: preferences.itemTextSize + 2
            font.bold: true
            color: darkone.textColour1
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
            color: darkone.textColour1
            opacity: 0.5
            smooth: true
        }
        CheckBox {
            id: screenLightCheckBox
            property int index: prefsEffectsText.index + 1
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: darkone.screenLight
            text: qsTr("screen lighting")
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            smooth: true

            onCheckedChanged: {
                overlayScreenLight.visible = darkone.opacity == 0 ? false : true
                darkone.screenLight = checked;
            }

            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: overlayScaleSliderItem
            KeyNavigation.tab: screenLightOpacitySliderItem
        }
        SliderItem {
            id: screenLightOpacitySliderItem
            property int index: prefsEffectsText.index + 2
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            fgColour1: darkone.colour3
            fgColour2: darkone.colour4
            bgColour1: "white"
            bgColour2: "white"
            textPrefix: qsTr("screen light opacity")
            textSuffix: DarkoneJS.round(100 * darkone.screenLightOpacity, 0) + "%"
            sliderWidth: 50
            slidePercentage: 5
            minimum: 0
            maximum: 1
            value: darkone.screenLightOpacity;

            onValueChanged: darkone.screenLightOpacity = DarkoneJS.round(value, 2);

            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: screenLightCheckBox
            KeyNavigation.tab: backLightCheckBox
        }
        CheckBox {
            id: backLightCheckBox
            property int index: prefsEffectsText.index + 3
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: darkone.backLight
            text: qsTr("back lighting")
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            smooth: true

            onCheckedChanged: {
                overlayBackLight.visible = darkone.opacity == 0 ? false : true
                darkone.backLight = checked;
            }

            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: screenLightOpacitySliderItem
            KeyNavigation.tab: backLightOpacitySliderItem
        }
        SliderItem {
            id: backLightOpacitySliderItem
            property int index: prefsEffectsText.index + 4
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            fgColour1: darkone.colour3
            fgColour2: darkone.colour4
            bgColour1: "white"
            bgColour2: "white"
            textPrefix: qsTr("back light opacity  ")
            textSuffix: DarkoneJS.round(100 * darkone.backLightOpacity, 0) + "%"
            sliderWidth: 50
            slidePercentage: 4
            minimum: 0
            maximum: 1
            value: darkone.backLightOpacity;

            onValueChanged: darkone.backLightOpacity = DarkoneJS.round(value, 2);

            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: backLightCheckBox
            KeyNavigation.tab: launchFlashCheckBox
        }
        CheckBox {
            id: launchFlashCheckBox
            property int index: prefsEffectsText.index + 5
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: darkone.launchFlash
            text: qsTr("launch flash?")
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            smooth: true

            onCheckedChanged: { darkone.launchFlash = checked; }

            KeyNavigation.up: KeyNavigation.backtab
            KeyNavigation.down: KeyNavigation.tab
            KeyNavigation.backtab: backLightCheckBox
            KeyNavigation.tab: launchZoomCheckBox
        }
        CheckBox {
            id: launchZoomCheckBox
            property int index: prefsEffectsText.index + 6
            height: parent.itemHeight
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            anchors.right: parent.right
            anchors.rightMargin: 10
            checked: darkone.launchZoom
            text: qsTr("launch zoom?")
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            smooth: true

            onCheckedChanged: { darkone.launchZoom = checked; }

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
            property int index: 15
            opacity: 1.0
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            text: qsTr("colour scheme")
            font.pixelSize: preferences.itemTextSize + 2
            font.bold: true
            color: darkone.textColour1
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
            color: darkone.textColour1
            opacity: 0.5
            smooth: true
        }
        CheckableGroup { id: checkGroup }
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
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2

            onCheckedChanged: { 
                if ( checked ) { darkone.colourScheme = "dark"; }
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
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2

            onCheckedChanged: {
                if ( checked ) { darkone.colourScheme = "metal"; }
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
            property int index: 18
            opacity: 1.0
            anchors.top: parent.top
            anchors.topMargin: index * (parent.itemHeight + parent.itemSpacing)
            anchors.left: parent.left
            anchors.leftMargin: 10
            text: qsTr("backend")
            font.pixelSize: preferences.itemTextSize + 2
            font.bold: true
            color: darkone.textColour1
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
            color: darkone.textColour1
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
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            items: viewer.cliParamNames()
            image: "../images/arrow.png"
            imageWidth: 14
            imageRotation: 0

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
            font.pixelSize: preferences.itemTextSize
            font.bold: false
            color: darkone.textColour1
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
            font.pixelSize: preferences.itemTextSize + 1
            color: darkone.textColour1
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
            textSize: preferences.itemTextSize
            textColour: darkone.textColour1
            activeColour: darkone.textColour2
            textPrefix: "value: "
            image: "../images/arrow.png"
            imageWidth: 14
            imageRotation: 0

            onSelect: {
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
            KeyNavigation.backtab: backendParamNamesCycleItem
            KeyNavigation.tab: sortByNameCheckBox
        }
    }
    } // focusScope


/***
* toolbar
*/
    FocusScope {
        id: toolbarFocusScope
        focus: false // darkoneFocusScope
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.right: parent.right
        width: toolbar.width
        height: toolbar.height

        onActiveFocusChanged: {
            debug2 && focus && DarkoneJS.inFocus();
//            searchTextInput.forceActiveFocus(); // hack!! bug?
        }

    Rectangle {
        id: toolbarBorder
        z: toolbar.z + 15
        anchors.bottom: parent.bottom
        anchors.bottomMargin: toolbar.height + 1
        anchors.left: parent.left
        anchors.leftMargin: 1
        height: 2
        width: darkone.width - 2
        color: darkone.textColour2
        visible: parent.activeFocus || searchTextInput.activeFocus
    }

    Rectangle {
        id: toolbar
        z: 4
        anchors.horizontalCenter: parent.horizontalCenter
        anchors.bottom: parent.bottom
        anchors.bottomMargin: 0
        width: DarkoneJS.baseWidth * DarkoneJS.scaleFactorX()
        height: 36
        opacity: 0.75
        smooth: true
        state: "shown"
        gradient: Gradient {
            GradientStop { position: 0.0; color: darkone.colour1 }
            GradientStop { position: 0.2; color: darkone.colour2 }
            GradientStop { position: 0.7; color: darkone.colour3 }
            GradientStop { position: 1.0; color: darkone.colour4 }
        }

        transitions: [
            Transition {
                from: "hidden"
                to: "shown"
                SequentialAnimation {
                    ParallelAnimation {
                        PropertyAnimation { target: toolbar; property: "anchors.bottomMargin"; from: -(toolbar.height - 2); to: 0; duration: 500; easing.type: Easing.OutCubic }
                        PropertyAnimation { target: gameListView; property: "anchors.bottomMargin"; from: 2 + gameListView.itemHeight; to: toolbar.height + gameListView.itemHeight; duration: 500; easing.type: Easing.OutCubic }
                    }
                    PropertyAnimation { target: toolbarBorder; property: "opacity"; from: 0; to: 1.0; duration: 0; }
                }
            },
            Transition {
                from: "shown"
                to: "hidden"
                SequentialAnimation {
                    PropertyAnimation { target: toolbarBorder; property: "opacity"; from: 1.0; to: 0; duration: 0; }
                    ParallelAnimation {
                        PropertyAnimation { target: toolbar; property: "anchors.bottomMargin"; from: 0; to: -(toolbar.height - 2); duration: 500; easing.type: Easing.OutCubic }
                        PropertyAnimation { target: gameListView; property: "anchors.bottomMargin"; from: toolbar.height + gameListView.itemHeight; to: 2 + gameListView.itemHeight; duration: 500; easing.type: Easing.OutCubic }
                    }
                }
            }
        ]

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
            onEntered: { darkone.toolbarHidden && !DarkoneJS.inGame && DarkoneJS.toolbarToggle(); hideToolbarTimer.stop(); }
            onExited: { hideToolbarTimer.start(); }
            onPositionChanged: { if (!darkone.keepLightOn) {
                                     lightOutTimer.restart();
                                     lightOutScreenTimer.restart();
                                 }
                                 if (darkone.lightOut)
                                     DarkoneJS.lightToggle(1);
            }
        }
        // toolbar key events
        Keys.onPressed: {
            debug2 && console.log("[keys] gameListView: '" + DarkoneJS.keyEvent2String(event) + "'")
            if ( darkone.toolbarHidden )
                console.log("[toolbar] error: key press in hidden state")
            else {
                switch ( event.key ) {
                    case Qt.Key_Tab: {
                        if ( event.modifiers & Qt.ShiftModifier ) {
                            if ( !darkone.listHidden )
                                gameListView.forceActiveFocus();
                            else
                                overlay.forceActiveFocus();
                        } else
                            overlay.forceActiveFocus();
                        event.accepted = true;
                        break;
                    }   
                    default: {
                        if ( DarkoneJS.validateKey(event.text) ) {
                            searchTextInput.forceActiveFocus();
                            searchTextInput.text += event.text;
                            event.accepted = true;
                            break;
                        } else if ( DarkoneJS.validateSpecialKey(event.text) ) {
                            searchTextInput.forceActiveFocus();
                            switch ( event.text ) {
                                case "\b": {
                                    if ( searchTextInput.text.length > 0)
                                        searchTextInput.text = searchTextInput.text.substring(0, searchTextInput.text.length - 1)
                                    event.accepted = true;
                                    break;
;                               }
                            }
                        }
                    }
                    break;
                } // switch
            }
        }

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
                    onEntered: { darkone.toolbarAutoHide && darkone.toolbarHidden && !DarkoneJS.inGame && DarkoneJS.toolbarToggle(); hideToolbarTimer.stop(); parent.opacity = 1.0 }
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
                    focus: true // toolbarFocusScope

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
            anchors.bottom: toolbar.bottom
            anchors.bottomMargin: (toolbar.height - height) / 2
            anchors.left: searchBox.right
            anchors.leftMargin: 15
            fillMode: Image.PreserveAspectFit
            opacity: 0.75
            rotation: darkone.listHidden ? 90 : 270
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
                    debug && console.log("[preferencesButton clicked] state: '" + preferences.state + "'")
                    debug && DarkoneJS.info("[preferencesButton clicked]", preferences)
                    parent.opacity = 1.0;
                    if (preferences.state == "shown") {
                        preferences.state = "hidden";
                    } else {
                        preferences.state = "shown";
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
                }
            }
        }
        Text {
            id: fpsText
            anchors.verticalCenter: parent.verticalCenter
            anchors.left: fullScreenToggleButton.right
            anchors.leftMargin: 15
            color: darkone.textColour1
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

                CursorShapeArea {
                    anchors.fill: parent
                    cursorShape: Qt.CrossCursor
                }
                MouseArea {
                    anchors.fill: parent
                    hoverEnabled: true
                    onEntered: { parent.opacity = 1.0
                                 parent.parent.opacity = 1.0
                                 overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 1.0;
                    }
                    onExited: { parent.opacity = 0.75
                                parent.parent.opacity = 0.5
                                overlayStateBlock.opacity = (lightOnAnimation.running || darkone.lightOut) ? 0 : 0.5;
                    }
                    onClicked: { if (!darkone.ignoreLaunch) {
                                     gameListView.positionViewAtIndex(darkone.lastIndex, ListView.Center);
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
    }
} // darkone
} // darkoneFocusScope
