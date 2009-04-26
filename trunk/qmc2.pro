greaterThan(QT_MAJOR_VERSION, 3) {
	greaterThan(QT_MINOR_VERSION, 3) {
		isEmpty(TARGET):TARGET = qmc2
		CONFIG += qtestlib
		QT += xml
		TEMPLATE = app
		INCLUDEPATH += minizip/
		FORMS += qmc2main.ui \
			options.ui \
			dbrowser.ui \
			about.ui \
			welcome.ui \
			imgcheck.ui \
			sampcheck.ui \
			keyseqscan.ui \
			joyfuncscan.ui \
			toolexec.ui \
			itemselect.ui \
			romalyzer.ui \
			romstatusexport.ui \
			messdevcfg.ui \
			direditwidget.ui \
			fileeditwidget.ui \
			arcade/arcadesetupdialog.ui
		SOURCES += qmc2main.cpp \
			options.cpp \
			dbrowser.cpp \
			about.cpp \
			welcome.cpp \
			imgcheck.cpp \
			sampcheck.cpp \
			keyseqscan.cpp \
			toolexec.cpp \
			itemselect.cpp \
			romalyzer.cpp \
			gamelist.cpp \
			procmgr.cpp \
			preview.cpp \
			flyer.cpp \
			cabinet.cpp \
			controller.cpp \
			marquee.cpp \
			title.cpp \
			emuopt.cpp \
			joystick.cpp \
			joyfuncscan.cpp \
			romstatusexport.cpp \
			messdevcfg.cpp \
			direditwidget.cpp \
			fileeditwidget.cpp \
			minizip/ioapi.c \
			minizip/unzip.c \
			arcade/arcadeview.cpp \
			arcade/arcadeitem.cpp \
			arcade/arcademenuitem.cpp \
			arcade/arcadescene.cpp \
			arcade/arcademenuscene.cpp \
			arcade/arcadesettings.cpp \
			arcade/arcadescreenshotsaverthread.cpp \
			arcade/arcadesetupdialog.cpp
		HEADERS += qmc2main.h \
			options.h \
			dbrowser.h \
			about.h \
			welcome.h \
			imgcheck.h \
			sampcheck.h \
			keyseqscan.h \
			toolexec.h \
			itemselect.h \
			romalyzer.h \
			gamelist.h \
			procmgr.h \
			preview.h \
			flyer.h \
			cabinet.h \
			controller.h \
			marquee.h \
			title.h \
			emuopt.h \
			joystick.h \
			joyfuncscan.h \
			romstatusexport.h \
			messdevcfg.h \
			direditwidget.h \
			fileeditwidget.h \
			macros.h \
			minizip/ioapi.h \
			minizip/unzip.h \
			arcade/arcadeview.h \
			arcade/arcadeitem.h \
			arcade/arcademenuitem.h \
			arcade/arcadescene.h \
			arcade/arcademenuscene.h \
			arcade/arcadesettings.h \
			arcade/arcadescreenshotsaverthread.h \
			arcade/arcadesetupdialog.h
		PRECOMPILED_HEADER = qmc2_prefix.h
		TRANSLATIONS += data/lng/qmc2_us.ts \
			data/lng/qmc2_de.ts \
			data/lng/qmc2_pl.ts \
			data/lng/qmc2_fr.ts
		RESOURCES += qmc2.qrc
		QMAKE_MAKEFILE = Makefile.qmake
		# produce pretty (silent) compile output
		greaterThan(QMC2_PRETTY_COMPILE, 0) { 
			!isEmpty(QMAKE_CXX):QMAKE_CXX = @echo [C++ ] $< && $$QMAKE_CXX
			!isEmpty(QMAKE_CC):QMAKE_CC = @echo [CC\ \ ] $< && $$QMAKE_CC
			!isEmpty(QMAKE_LINK):QMAKE_LINK = @echo [LINK] $@ && $$QMAKE_LINK
			!isEmpty(QMAKE_MOC):QMAKE_MOC = @echo [MOC ] `echo $@ | sed -e \'s/moc_//g\' | sed -e \'s/.cpp/.h/g\'` && $$QMAKE_MOC
			!isEmpty(QMAKE_UIC):QMAKE_UIC = @echo [UIC ] $< && $$QMAKE_UIC
			!isEmpty(QMAKE_RCC):QMAKE_RCC = @echo [RCC ] $< && $$QMAKE_RCC
		}
		macx {
			OBJECTIVE_SOURCES += SDLMain_tmpl.m
			HEADERS += SDLMain_tmpl.h
			LIBS += -framework SDL
			CONFIG += x86 ppc
			QMAKE_INFO_PLIST = macx/Info.plist
		} else {
			!win32 {
				LIBS += -lSDL
			}
		}
		win32 {
			CONFIG += embed_manifest_exe
		}
	} else {
		error(Qt $$QT_VERSION is insufficient -- Qt 4.4.0+ required)
	}
} else {
	error(Qt $$QT_VERSION is insufficient -- Qt 4.4.0+ required)
}
