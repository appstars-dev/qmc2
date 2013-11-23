#ifndef KEYSEQUENCES_H
#define KEYSEQUENCES_H

// common key-sequences
#define QMC2_ARCADE_ADD_COMMON_KEYSEQUENCES(stringList)         (stringList) << "Enter" \
                                                                             << "Return" \
                                                                             << "F11" \
                                                                             << "Alt+Enter" \
                                                                             << "Alt+Return"

// common key-sequence descriptions
#define QMC2_ARCADE_ADD_COMMON_DESCRIPTIONS(stringList)         (stringList) << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("Toggle full-screen / windowed mode")
// toxicwaste-specific key-sequences
#define QMC2_ARCADE_ADD_TOXIXCWASTE_KEYSEQUENCES(stringList)    (stringList) << "Up" \
                                                                             << "Down" \
                                                                             << "Left" \
                                                                             << "Right" \
                                                                             << "PgUp" \
                                                                             << "PgDown" \
                                                                             << "Home" \
                                                                             << "End" \
                                                                             << "Ctrl+F" \
                                                                             << "Ctrl+M" \
                                                                             << "Ctrl+O" \
                                                                             << "Ctrl+P" \
                                                                             << "Esc" \
                                                                             << "Ctrl+X" \
                                                                             << "Ctrl+Backspace"
// toxicwaste-specific key-sequence dscriptions
#define QMC2_ARCADE_ADD_TOXIXCWASTE_DESCRIPTIONS(stringList)    (stringList) << QObject::tr("Cursor up") \
                                                                             << QObject::tr("Cursor down") \
                                                                             << QObject::tr("Cursor left") \
                                                                             << QObject::tr("Cursor right") \
                                                                             << QObject::tr("Page up") \
                                                                             << QObject::tr("Page down") \
                                                                             << QObject::tr("Start of list") \
                                                                             << QObject::tr("End of list") \
                                                                             << QObject::tr("Focus search box") \
                                                                             << QObject::tr("Toggle menu-bar") \
                                                                             << QObject::tr("Toggle preferences") \
                                                                             << QObject::tr("Start emulation") \
                                                                             << QObject::tr("Exit") \
                                                                             << QObject::tr("Exit") \
                                                                             << QObject::tr("Flip cabinet / game-card")
// darkone-specific key-sequences
#define QMC2_ARCADE_ADD_DARKONE_KEYSEQUENCES(stringList)        (stringList) << "Ctrl+Up" \
                                                                             << "Ctrl+Down" \
                                                                             << "Left" \
                                                                             << "Right" \
                                                                             << "Up" \
                                                                             << "Down" \
                                                                             << "Enter" \
                                                                             << "Esc" \
                                                                             << "Ctrl+Shift+Up" \
                                                                             << "Ctrl+Shift+Down" \
                                                                             << "PgUp" \
                                                                             << "PgDown" \
                                                                             << "Home" \
                                                                             << "End" \
                                                                             << "1" \
                                                                             << "2" \
                                                                             << "Tab" \
                                                                             << "Ctrl+Right" \
                                                                             << "Shift+Tab" \
                                                                             << "Ctrl+Left" \
                                                                             << "Plus" \
                                                                             << "Minus" \
                                                                             << "Ctrl+P" \
                                                                             << "Ctrl+S" \
                                                                             << "Ctrl+O" \
                                                                             << "Ctrl+L" \
                                                                             << "Ctrl+T" \
                                                                             << "Alt+F" \
                                                                             << "Ctrl+Q"
// darkone-specific key-sequence descriptions
#define QMC2_ARCADE_ADD_DARKONE_DESCRIPTIONS(stringList)       (stringList)  << QObject::tr("[context] Previous component / List page up / Info page up") \
                                                                             << QObject::tr("[context] Next component / List page down / Info page down") \
                                                                             << QObject::tr("[context] Hide list / Previous item / Slide left / Cycle backwards") \
                                                                             << QObject::tr("[context] Show list / Next item / Slide right / Cycle forwards") \
                                                                             << QObject::tr("[context] Show toolbar / List up / Info up / Previous widget") \
                                                                             << QObject::tr("[context] Hide toolbar / List down / Info down / Next widget") \
                                                                             << QObject::tr("[context] Select / Set / Toggle details / Start emulation") \
                                                                             << QObject::tr("[context] Abort game launch / Hide preferences") \
                                                                             << QObject::tr("[context] Zoom in / List top") \
                                                                             << QObject::tr("[context] Zoom out / List bottom") \
                                                                             << QObject::tr("[context] List page up / Flick page up") \
                                                                             << QObject::tr("[context] List page down / Flick page down") \
                                                                             << QObject::tr("[context] List top") \
                                                                             << QObject::tr("[context] List bottom") \
                                                                             << QObject::tr("[context] Set primary display data item") \
                                                                             << QObject::tr("[context] Set secondary display data item") \
                                                                             << QObject::tr("[global] Next widget") \
                                                                             << QObject::tr("[global] Next widget") \
                                                                             << QObject::tr("[global] Previous widget") \
                                                                             << QObject::tr("[global] Previous widget") \
                                                                             << QObject::tr("[global] Zoom in") \
                                                                             << QObject::tr("[global] Zoom out") \
                                                                             << QObject::tr("[global] Start emulation") \
                                                                             << QObject::tr("[global] Search") \
                                                                             << QObject::tr("[global] Toggle preferences") \
                                                                             << QObject::tr("[global] Toggle list") \
                                                                             << QObject::tr("[global] Toggle toolbar") \
                                                                             << QObject::tr("[global] Toggle full-screen / windowed mode") \
                                                                             << QObject::tr("[global] Exit")

#endif // KEYSEQUENCES_H
