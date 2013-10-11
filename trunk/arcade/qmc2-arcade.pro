VERSION = 0.4

# Add more folders to ship with the application, here
folder_01.source = qml/ToxicWaste
folder_01.target = qml
folder_02.source = qml/darkone
folder_02.target = qml
DEPLOYMENTFOLDERS = folder_01 folder_02

# Additional import path used to resolve QML modules in Creator's code model
QML_IMPORT_PATH =

# If your application uses the Qt Mobility libraries, uncomment the following
# lines and add the respective components to the MOBILITY variable.
# CONFIG += mobility
# MOBILITY +=

# Speed up launching on MeeGo/Harmattan when using applauncherd daemon
# CONFIG += qdeclarative-boostable

# The .cpp file which was generated for your project. Feel free to hack it.
SOURCES += main.cpp \
    tweakedqmlappviewer.cpp \
    imageprovider.cpp \
    infoprovider.cpp \
    arcadesettings.cpp \
    gameobject.cpp \
    consolewindow.cpp \
    processmanager.cpp \
    joystick.cpp \
    pointer.cpp \
    ../zlib/zutil.c \
    ../zlib/uncompr.c \
    ../zlib/trees.c \
    ../zlib/inftrees.c \
    ../zlib/inflate.c \
    ../zlib/inffast.c \
    ../zlib/infback.c \
    ../zlib/gzwrite.c \
    ../zlib/gzread.c \
    ../zlib/gzlib.c \
    ../zlib/gzclose.c \
    ../zlib/deflate.c \
    ../zlib/crc32.c \
    ../zlib/compress.c \
    ../zlib/adler32.c \
    ../minizip/zip.c \
    ../minizip/unzip.c \
    ../minizip/ioapi.c \
    keyeventfilter.cpp \
    keysequencemap.cpp \
    joyfunctionmap.cpp \
    joystickmanager.cpp

# Please do not modify the following two lines. Required for deployment.
include(qmlapplicationviewer/qmlapplicationviewer.pri)
qtcAddDeployment()

OTHER_FILES += \
    qml/ToxicWaste/1.1/ToxicWaste.qml \
    qml/ToxicWaste/1.1/ToxicWaste.js \
    qml/ToxicWaste/1.1/BackgroundAnimation.qml \
    qml/ToxicWaste/2.0/ToxicWaste.qml \
    qml/ToxicWaste/2.0/ToxicWaste.js \
    qml/ToxicWaste/2.0/BackgroundAnimation.qml \
    qml/darkone/1.1/darkone.qml \
    qml/darkone/1.1/darkone.js \
    qml/darkone/2.0/darkone.qml \
    qml/darkone/2.0/darkone.js \
    ../zlib/README.zlib

HEADERS += \
    tweakedqmlappviewer.h \
    imageprovider.h \
    infoprovider.h \
    arcadesettings.h \
    macros.h \
    gameobject.h \
    consolewindow.h \
    processmanager.h \
    emulatoroption.h \
    joystick.h \
    wheel.h \
    pointer.h \
    ../zlib/zutil.h \
    ../zlib/zlib.h \
    ../zlib/zconf.h \
    ../zlib/trees.h \
    ../zlib/inftrees.h \
    ../zlib/inflate.h \
    ../zlib/inffixed.h \
    ../zlib/inffast.h \
    ../zlib/gzguts.h \
    ../zlib/deflate.h \
    ../zlib/crc32.h \
    ../minizip/zip.h \
    ../minizip/unzip.h \
    ../minizip/ioapi.h \
    ../minizip/crypt.h \
    keyeventfilter.h \
    keysequences.h \
    keysequencemap.h \
    joyfunctionmap.h \
    joystickmanager.h

DEFINES += QMC2_ARCADE_VERSION=$$VERSION

RESOURCES += qmc2-arcade-common.qrc
greaterThan(QT_MAJOR_VERSION, 4) {
    RESOURCES += qmc2-arcade-2-0.qrc
} else {
    RESOURCES += qmc2-arcade-1-1.qrc
}

evil_hack_to_fool_lupdate {
    SOURCES += qml/ToxicWaste/1.1/ToxicWaste.qml \
               qml/ToxicWaste/1.1/ToxicWaste.js \
               qml/ToxicWaste/1.1/animations/BackgroundAnimation.qml \
               qml/ToxicWaste/2.0/ToxicWaste.qml \
               qml/ToxicWaste/2.0/ToxicWaste.js \
               qml/ToxicWaste/2.0/animations/BackgroundAnimation.qml \
               qml/darkone/1.1/darkone.qml \
               qml/darkone/1.1/darkone.js \
               qml/darkone/2.0/darkone.qml \
               qml/darkone/2.0/darkone.js
}

TRANSLATIONS += translations/qmc2-arcade_de.ts \
    translations/qmc2-arcade_es.ts \
    translations/qmc2-arcade_el.ts \
    translations/qmc2-arcade_it.ts \
    translations/qmc2-arcade_fr.ts \
    translations/qmc2-arcade_pl.ts \
    translations/qmc2-arcade_pt.ts \
    translations/qmc2-arcade_ro.ts \
    translations/qmc2-arcade_sv.ts \
    translations/qmc2-arcade_us.ts

greaterThan(SVN_REV, 0) {
    DEFINES += QMC2_ARCADE_SVN_REV=$$SVN_REV
}

isEmpty(QMC2_ARCADE_JOYSTICK): QMC2_ARCADE_JOYSTICK = 1
greaterThan(QMC2_ARCADE_JOYSTICK, 0) {
    DEFINES += QMC2_ARCADE_ENABLE_JOYSTICK
}

isEmpty(QMC2_ARCADE_QML_IMPORT_PATH): QMC2_ARCADE_QML_IMPORT_PATH = imports
DEFINES += QMC2_ARCADE_QML_IMPORT_PATH=$$QMC2_ARCADE_QML_IMPORT_PATH

macx {
    OBJECTIVE_SOURCES += ../SDLMain_tmpl.m
    HEADERS += ../SDLMain_tmpl.h
    LIBS += -framework SDL -framework Cocoa
    ICON = images/qmc2-arcade.icns
    contains(DEFINES, QMC2_ARCADE_MAC_UNIVERSAL): CONFIG += x86 ppc
    QMAKE_INFO_PLIST = Info.plist
    QT += opengl
} else {
    !win32 {
        LIBS += -lSDL
    } else {
        contains(DEFINES, QMC2_ARCADE_MINGW) {
                CONFIG += windows
                LIBS += -lSDLmain -lSDL.dll -lSDL
        } else {
                CONFIG += embed_manifest_exe windows
        }
        RC_FILE = qmc2-arcade.rc
    }
}

greaterThan(QT_MAJOR_VERSION, 4) {
    QT += gui quick qml
}

QT += svg

INCLUDEPATH += ../zlib
