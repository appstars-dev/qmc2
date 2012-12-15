import QtQuick 1.1
import "ToxicWaste.js" as ToxicWaste

Item {
    id: animationItem
    anchors.fill: parent
    property bool running: false

    function randomize(min, max) {
        return Math.floor(Math.random() * (max - min)) + min;
    }

    Image {
        id: purpleBubble
        source: "images/purple_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        x: randomize(0, toxicWasteMain.width - purpleBubble.width)
        y: randomize(0, toxicWasteMain.height - purpleBubble.height)
        smooth: true
        SequentialAnimation {
            id: purpleBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                SequentialAnimation {
                    NumberAnimation { id: pn1x; target: purpleBubble; property: "x"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - purpleBubble.width); duration: randomize(3000, 10000) }
                    NumberAnimation { id: pn2x; target: purpleBubble; property: "x"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - purpleBubble.width); duration: randomize(3000, 10000) }
                }
                SequentialAnimation {
                    NumberAnimation { id: pn1y; target: purpleBubble; property: "y"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - purpleBubble.height); duration: randomize(3000, 10000) }
                    NumberAnimation { id: pn2y; target: purpleBubble; property: "y"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - purpleBubble.height); duration: randomize(3000, 10000) }
                }
            }
            ScriptAction {
                script: {
                    pn1x.to = randomize(0, toxicWasteMain.width - purpleBubble.width);
                    pn1x.duration = randomize(3000, 10000);
                    pn2x.to = randomize(0, toxicWasteMain.width - purpleBubble.width);
                    pn2x.duration = randomize(3000, 10000);
                    pn1y.to = randomize(0, toxicWasteMain.height - purpleBubble.height);
                    pn1y.duration = randomize(3000, 10000);
                    pn2y.to = randomize(0, toxicWasteMain.height - purpleBubble.height);
                    pn2y.duration = randomize(3000, 10000);
                    purpleBubbleAnimation.restart();
                }
            }
        }
    }
    Image {
        id: blueBubble
        source: "images/blue_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        x: randomize(0, toxicWasteMain.width - blueBubble.width)
        y: randomize(0, toxicWasteMain.height - blueBubble.height)
        smooth: true
        SequentialAnimation {
            id: blueBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                SequentialAnimation {
                    NumberAnimation { id: bn1x; target: blueBubble; property: "x"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - blueBubble.width); duration: randomize(3000, 10000) }
                    NumberAnimation { id: bn2x; target: blueBubble; property: "x"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - blueBubble.width); duration: randomize(3000, 10000) }
                }
                SequentialAnimation {
                    NumberAnimation { id: bn1y; target: blueBubble; property: "y"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - blueBubble.height); duration: randomize(3000, 10000) }
                    NumberAnimation { id: bn2y; target: blueBubble; property: "y"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - blueBubble.height); duration: randomize(3000, 10000) }
                }
            }
            ScriptAction {
                script: {
                    bn1x.to = randomize(0, toxicWasteMain.width - blueBubble.width);
                    bn1x.duration = randomize(3000, 10000);
                    bn2x.to = randomize(0, toxicWasteMain.width - blueBubble.width);
                    bn2x.duration = randomize(3000, 10000);
                    bn1y.to = randomize(0, toxicWasteMain.height - blueBubble.height);
                    bn1y.duration = randomize(3000, 10000);
                    bn2y.to = randomize(0, toxicWasteMain.height - blueBubble.height);
                    bn2y.duration = randomize(3000, 10000);
                    blueBubbleAnimation.restart();
                }
            }
        }
    }
    Image {
        id: greenBubble
        source: "images/green_bubble.png"
        scale: ToxicWaste.scaleFactorX()
        x: randomize(0, toxicWasteMain.width - greenBubble.width)
        y: randomize(0, toxicWasteMain.height - greenBubble.height)
        smooth: true
        SequentialAnimation {
            id: greenBubbleAnimation
            running: animationItem.running
            ParallelAnimation {
                SequentialAnimation {
                    NumberAnimation { id: gn1x; target: greenBubble; property: "x"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - greenBubble.width); duration: randomize(3000, 10000) }
                    NumberAnimation { id: gn2x; target: greenBubble; property: "x"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.width - greenBubble.width); duration: randomize(3000, 10000) }
                }
                SequentialAnimation {
                    NumberAnimation { id: gn1y; target: greenBubble; property: "y"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - greenBubble.height); duration: randomize(3000, 10000) }
                    NumberAnimation { id: gn2y; target: greenBubble; property: "y"; easing.type: Easing.InOutBounce; to: randomize(0, toxicWasteMain.height - greenBubble.height); duration: randomize(3000, 10000) }
                }
            }
            ScriptAction {
                script: {
                    gn1x.to = randomize(0, toxicWasteMain.width - greenBubble.width);
                    gn1x.duration = randomize(3000, 10000);
                    gn2x.to = randomize(0, toxicWasteMain.width - greenBubble.width);
                    gn2x.duration = randomize(3000, 10000);
                    gn1y.to = randomize(0, toxicWasteMain.height - greenBubble.height);
                    gn1y.duration = randomize(3000, 10000);
                    gn2y.to = randomize(0, toxicWasteMain.height - greenBubble.height);
                    gn2y.duration = randomize(3000, 10000);
                    greenBubbleAnimation.restart();
                }
            }
        }
    }
}
