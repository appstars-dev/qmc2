/****************************************************************************
 **
 ** Copyright (C) 2012 Digia Plc and/or its subsidiary(-ies).
 ** Contact: http://www.qt-project.org/legal
 **
 ** This file is part of the QtDeclarative module of the Qt Toolkit.
 **
 ** $QT_BEGIN_LICENSE:LGPL$
 ** Commercial License Usage
 ** Licensees holding valid commercial Qt licenses may use this file in
 ** accordance with the commercial license agreement provided with the
 ** Software or, alternatively, in accordance with the terms contained in
 ** a written agreement between you and Digia.  For licensing terms and
 ** conditions see http://qt.digia.com/licensing.  For further information
 ** use the contact form at http://qt.digia.com/contact-us.
 **
 ** GNU Lesser General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU Lesser
 ** General Public License version 2.1 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.LGPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU Lesser General Public License version 2.1 requirements
 ** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
 **
 ** In addition, as a special exception, Digia gives you certain additional
 ** rights.  These rights are described in the Digia Qt LGPL Exception
 ** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
 **
 ** GNU General Public License Usage
 ** Alternatively, this file may be used under the terms of the GNU
 ** General Public License version 3.0 as published by the Free Software
 ** Foundation and appearing in the file LICENSE.GPL included in the
 ** packaging of this file.  Please review the following information to
 ** ensure the GNU General Public License version 3.0 requirements will be
 ** met: http://www.gnu.org/copyleft/gpl.html.
 **
 **
 ** $QT_END_LICENSE$
 **
 ****************************************************************************/

import QtQuick 1.0

Item {
    id: slider; width: 400; height: 16

    // value is read/write.
    property real value: 1
    onValueChanged: updatePos();
    property real maximum: 1
    property real minimum: 1
    property int xMax: width - handle.width - 4
    onXMaxChanged: updatePos();
    onMinimumChanged: updatePos();

    property string fgColour1: "lightgray"
    property string fgColour2: "gray"
    property string bgColour1: "black"
    property string bgColour2: "black"
    property string activeColour: "red"
    property int slidePercentage: 1

    function updatePos() {
        if (maximum > minimum) {
            var pos = 2 + (value - minimum) * slider.xMax / (maximum - minimum);
            pos = Math.min(pos, width - handle.width - 2);
            pos = Math.max(pos, 2);
            handle.x = pos;
        } else {
            handle.x = 2;
        }
    }
    function slide(change) {
        value = Math.max(minimum, value + (change / 100) * (maximum - minimum))
    }

    Rectangle {
        id: sliderBackground
        anchors.fill: parent
        border.width: slider.activeFocus ? 2 : 0
        border.color: parent.activeColour
        radius: 3
        gradient: Gradient {
            GradientStop { position: 0.0; color: bgColour1 }
            GradientStop { position: 1.0; color: bgColour2 }
        }
    }

    Rectangle {
        id: handle; smooth: true
        y: 1; width: 15; height: slider.height - 2; radius: 4
        gradient: Gradient {
            GradientStop { position: 0.0; color: fgColour1 }
            GradientStop { position: 1.0; color: fgColour2 }
        }

        MouseArea {
            id: mouse
            anchors.fill: parent; drag.target: parent
            drag.axis: Drag.XAxis; drag.minimumX: 2; drag.maximumX: slider.xMax + 2
            onPositionChanged: { value = (maximum - minimum) * (handle.x-2) / slider.xMax + minimum; }
        }
    }
    Keys.onPressed: {
       switch ( event.key ) {
           case Qt.Key_Left: {
               slide(-slidePercentage)
               event.accepted = true;
               break;
           }
           case Qt.Key_Right: {
               slide(slidePercentage)
               event.accepted = true;
               break;
           }
       }
    }
}
