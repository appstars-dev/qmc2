var lastItemText;
var lastitemBackground;
var initializing = true;

function init() {
    viewer.loadSettings();

    gamelistModel.clear();
    for (var i = 1; i < 501; i++)
        gamelistModel.append({"name": "Item " + i, "id": i});

    initializing = false;
}

function baseWidth() {
    return 800;
}

function baseHeight() {
    return 600;
}

function scaleFactorX() {
    return toxicWasteMain.width / baseWidth();
}

function scaleFactorY() {
    return toxicWasteMain.height / baseHeight();
}

function itemEntered(itemText, itemBackground) {
    if ( !itemText.fontResized ) {
        if ( lastItemText != undefined )
            itemExited(lastItemText, lastitemBackground);
        lastItemText = itemText;
        lastitemBackground = itemBackground;
        itemText.fontResized = true;
        itemText.font.pixelSize += 5;
        itemBackground.opacity = 1;
    }
}

function itemExited(itemText, itemBackground) {
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 5;
        itemBackground.opacity = 0.7;
    }
}

function itemClicked(itemText, itemBackground) {
    if ( itemText.fontResized ) {
        itemText.fontResized = false;
        itemText.font.pixelSize -= 5;
        itemBackground.opacity = 0.7;
    }
}
