#ifndef _MACROS_H_
#define _MACROS_H_

#include <Qt>

// min/max of two constants
#define MAX(a, b)			(((a) > (b)) ? (a) : (b))
#define MIN(a, b)			(((a) < (b)) ? (a) : (b))

// determine file existance
#if !defined(Q_WS_WIN)
#include <unistd.h>
#define EXISTS(fn)			(access(fn, F_OK) == 0)
#endif

// this is strange, but it's the only solution I found
// to make a string out of a non-string constant
#define STR(s)				#s
#define XSTR(s)				STR(s)

// logger positions
#define QMC2_LOG_FRONTEND		1
#define QMC2_LOG_EMULATOR		2

// index positions of game list tabs
#define QMC2_GAMELIST_INDEX		0
#define QMC2_MACHINELIST_INDEX		QMC2_GAMELIST_INDEX
#define QMC2_SEARCH_INDEX		1
#define QMC2_FAVORITES_INDEX		2
#define QMC2_PLAYED_INDEX		3

// index positions of game detail & image checker tabs
#define QMC2_PREVIEW_INDEX		0
#define QMC2_FLYER_INDEX		1
#define QMC2_GAMEINFO_INDEX		2
#define QMC2_MACHINEINFO_INDEX		QMC2_GAMEINFO_INDEX
#define QMC2_EMUINFO_INDEX		3
#define QMC2_CONFIG_INDEX		4
#define QMC2_DEVICE_INDEX		5
#define QMC2_CABINET_INDEX		6
#define QMC2_CONTROLLER_INDEX		7
#define QMC2_MARQUEE_INDEX		8
#define QMC2_TITLE_INDEX		9
#define QMC2_MAWS_INDEX			10

// column to add game icon in game list
#define QMC2_ICON_INDEX			2

// index positions of gamelist view selector
#define QMC2_VIEW_DETAIL_INDEX		0
#define QMC2_VIEW_TREE_INDEX		1

// logical column indizes in gamelists
#define QMC2_GAMELIST_COLUMN_GAME	0
#define QMC2_GAMELIST_COLUMN_ICON	1
#define QMC2_GAMELIST_COLUMN_YEAR	2
#define QMC2_GAMELIST_COLUMN_MANU	3
#define QMC2_GAMELIST_COLUMN_NAME	4
#define QMC2_MACHINELIST_COLUMN_GAME	QMC2_GAMELIST_COLUMN_GAME
#define QMC2_MACHINELIST_COLUMN_ICON	QMC2_GAMELIST_COLUMN_ICON
#define QMC2_MACHINELIST_COLUMN_YEAR	QMC2_GAMELIST_COLUMN_YEAR
#define QMC2_MACHINELIST_COLUMN_MANU	QMC2_GAMELIST_COLUMN_MANU
#define QMC2_AMCHINELIST_COLUMN_NAME	QMC2_GAMELIST_COLUMN_NAME

// logical column indizes in emulator control panel
#define QMC2_EMUCONTROL_COLUMN_NUMBER	0
#define QMC2_EMUCONTROL_COLUMN_GAME	1
#define QMC2_EMUCONTROL_COLUMN_MACHINE	QMC2_EMUCONTROL_COLUMN_GAME
#define QMC2_EMUCONTROL_COLUMN_STATUS	2
#define QMC2_EMUCONTROL_COLUMN_LED0	3
#define QMC2_EMUCONTROL_COLUMN_LED1	4
#define QMC2_EMUCONTROL_COLUMN_PID	5
#define QMC2_EMUCONTROL_COLUMN_COMMAND	6

// logical column indizes of MESS device configurator
#define QMC2_DEVCONFIG_COLUMN_NAME	0
#define QMC2_DEVCONFIG_COLUMN_TYPE	1
#define QMC2_DEVCONFIG_COLUMN_TAG	2
#define QMC2_DEVCONFIG_COLUMN_EXT	3
#define QMC2_DEVCONFIG_COLUMN_FILE	4

// logical column indizes in emulator options
#define QMC2_EMUOPT_COLUMN_OPTION	0
#define QMC2_EMUOPT_COLUMN_VALUE	1

// SDLMAME output notifier FIFO
#define QMC2_SDLMAME_OUTPUT_FIFO	"/tmp/sdlmame_out"
#define QMC2_SDLMESS_OUTPUT_FIFO	QMC2_SDLMAME_OUTPUT_FIFO

// index positions of game list view stack
#define QMC2_VIEWGAMELIST_INDEX		0
#define QMC2_VIEWHIERARCHY_INDEX	1

// index positions of tabs in the options dialog
#define QMC2_OPTIONS_FRONTEND_INDEX		0
#define QMC2_OPTIONS_EMULATOR_INDEX		1
#define QMC2_OPTIONS_FE_GUI_INDEX		0
#define QMC2_OPTIONS_FE_FILES_INDEX		1
#define QMC2_OPTIONS_FE_GAMELIST_INDEX		2
#define QMC2_OPTIONS_FE_MACHINELIST_INDEX	QMC2_OPTIONS_FE_GAMELIST_INDEX
#define QMC2_OPTIONS_FE_SHORTCUTS_INDEX		3
#define QMC2_OPTIONS_FE_JOYSTICK_INDEX		4
#define QMC2_OPTIONS_FE_TOOLS_INDEX		5
#define QMC2_OPTIONS_EMU_CONFIG_INDEX		0
#define QMC2_OPTIONS_EMU_FILES_INDEX		1

// index positions of sort criteria combobox entries
#define QMC2_SORTCRITERIA_DESCRIPTION		0
#define QMC2_SORTCRITERIA_ROMSTATE		1
#define QMC2_SORTCRITERIA_YEAR			2
#define QMC2_SORTCRITERIA_MANUFACTURER		3
#define QMC2_SORTCRITERIA_GAMENAME		4
#define QMC2_SORTCRITERIA_MACHINENAME		QMC2_SORTCRITERIA_GAMENAME

// search window timeout in milliseconds
#define QMC2_SEARCH_TIMEOUT		2000

// buffer size for reading zip-files
#define QMC2_ZIP_BUFFER_SIZE		32768

// buffer size for reading regular files
#define QMC2_FILE_BUFFER_SIZE		65536

// process timeouts in msecs
#define QMC2_PROCESS_POLL_TIME		25

// update lists every how many milliseconds (for image & sample checkers)?
#define QMC2_CHECK_UPDATE		1000

// maximium length of a single filename/path in a ZIP archive
#define QMC2_MAX_PATH_LENGTH		1024

// search delay in milliseconds (so it doesn't hamper typing)
#define QMC2_SEARCH_DELAY		250

// animation timeout in milliseconds (controls animation speeds)
#define QMC2_ANIMATION_TIMEOUT		250

// button animation timeout (similar to animateClick() w/o signal emission)
#define QMC2_BUTTON_ANIMATION_TIMEOUT	50

// gamelist sort criteria
#define QMC2_SORT_BY_DESCRIPTION	0
#define QMC2_SORT_BY_ROM_STATE		1
#define QMC2_SORT_BY_YEAR		2
#define QMC2_SORT_BY_MANUFACTURER	3
#define QMC2_SORT_BY_NAME		4

// internal ROM state representations
#define QMC2_ROMSTATE_COUNT		5
#define QMC2_ROMSTATE_INT_C		0
#define QMC2_ROMSTATE_INT_M		1
#define QMC2_ROMSTATE_INT_U		2
#define QMC2_ROMSTATE_INT_I		3
#define QMC2_ROMSTATE_INT_N		4
#define QMC2_ROMSTATE_CHAR_C		'0'
#define QMC2_ROMSTATE_CHAR_M		'1'
#define QMC2_ROMSTATE_CHAR_U		'2'
#define QMC2_ROMSTATE_CHAR_I		'3'
#define QMC2_ROMSTATE_CHAR_N		'4'
#define QMC2_ROMSTATE_STRING_C		"0"
#define QMC2_ROMSTATE_STRING_M		"1"
#define QMC2_ROMSTATE_STRING_U		"2"
#define QMC2_ROMSTATE_STRING_I		"3"
#define QMC2_ROMSTATE_STRING_N		"4"

// ROM state filter responsiveness (update gamelist after how many milliseconds?)
#define QMC2_STATEFILTER_RESPONSIVENESS	1000

// responsiveness while reading the XML gamelist cache
#define QMC2_XMLCACHE_RESPONSIVENESS	50

// responsiveness while loading an additional information source
#define QMC2_INFOSOURCE_RESPONSIVENESS	50

// emulator option types
#define QMC2_EMUOPT_TYPE_UNKNOWN	0
#define QMC2_EMUOPT_TYPE_BOOL		1
#define QMC2_EMUOPT_TYPE_INT		2
#define QMC2_EMUOPT_TYPE_FLOAT		3
#define QMC2_EMUOPT_TYPE_STRING		4
#define QMC2_EMUOPT_TYPE_FILE		5
#define QMC2_EMUOPT_TYPE_DIRECTORY	6

// standard sizes
#define QMC2_ONE_KILOBYTE		1024
#define QMC2_ONE_MEGABYTE		1048576
#define QMC2_ONE_GIGABYTE		1073741824
#define QMC2_ONE_TERABYTE		1099511627776

// MAWS homepage URL
#define QMC2_MAWS_HOMEPAGE_URL		"http://maws.mameworld.info/"

// MAWS ROM set base URL (default)
#define QMC2_MAWS_BASE_URL		"http://maws.mameworld.info/maws/romset/%1"

// MAWS web cache size (in-memory, 8M fixed for now)
#define QMC2_MAWS_CACHE_SIZE		8 * QMC2_ONE_MEGABYTE

// maximum age of an MAWS disk-cache entry in seconds (24h for now)
#define QMC2_MAWS_MAX_CACHE_AGE		86400

// type conversions
#define QMC2_TO_UINT32(a)		((uchar)*((a) + 0) * (quint32)16777216ULL + \
					(uchar)*((a) + 1) * (quint32)65536ULL + \
					(uchar)*((a) + 2) * (quint32)256ULL + \
					(uchar)*((a) + 3))
#define QMC2_TO_UINT64(a)		((uchar)*((a) + 0) * (quint64)72057594037927936ULL + \
					(uchar)*((a) + 1) * (quint64)281474976710656ULL + \
					(uchar)*((a) + 2) * (quint64)1099511627776ULL + \
					(uchar)*((a) + 3) * (quint64)4294967296ULL + \
					(uchar)*((a) + 4) * (quint64)16777216ULL + \
					(uchar)*((a) + 5) * (quint64)65536ULL + \
					(uchar)*((a) + 6) * (quint64)256ULL + \
					(uchar)*((a) + 7))

// additional pre-compile checks
#define QMC2_USE_PHONON_API		(QT_VERSION >= 0x040400 && QMC2_PHONON == 1)

// audio player seek offset (in milliseconds)
#define QMC2_AUDIOPLAYER_SEEK_OFFSET	1000

// audio player fader timeout 
#define QMC2_AUDIOPLAYER_FADER_TIMEOUT  500

// audio player fader functions
#define QMC2_AUDIOPLAYER_FADER_PAUSE	0
#define QMC2_AUDIOPLAYER_FADER_PLAY	1

// QMC2 variant launcher specific stuff
#define QMC2_COMMAND_RUNONCE		"runonce"
#define QMC2_VARIANT_SDLMAME_NAME	"qmc2-sdlmame"
#define QMC2_VARIANT_SDLMAME_BUNDLE_ID	"net.arcadehits.qmc2." QMC2_VARIANT_SDLMAME_NAME
#define QMC2_VARIANT_SDLMAME_TITLE	tr("M.A.M.E. Catalog / Launcher II")
#define QMC2_VARIANT_SDLMESS_NAME	"qmc2-sdlmess"
#define QMC2_VARIANT_SDLMESS_BUNDLE_ID	"net.arcadehits.qmc2." QMC2_VARIANT_SDLMESS_NAME
#define QMC2_VARIANT_SDLMESS_TITLE	tr("M.E.S.S. Catalog / Launcher II")
#define QMC2_VARIANT_MAME_NAME		"qmc2-mame"
#define QMC2_VARIANT_MAME_BUNDLE_ID	"net.arcadehits.qmc2." QMC2_VARIANT_MAME_NAME
#define QMC2_VARIANT_MAME_TITLE		tr("M.A.M.E. Catalog / Launcher II")
#define QMC2_VARIANT_MESS_NAME		"qmc2-mess"
#define QMC2_VARIANT_MESS_BUNDLE_ID	"net.arcadehits.qmc2." QMC2_VARIANT_MESS_NAME
#define QMC2_VARIANT_MESS_TITLE		tr("M.E.S.S. Catalog / Launcher II")
#if defined(QMC2_SDLMAME)
#define QMC2_FRONTEND_PREFIX		QString("Frontend/qmc2-sdlmame/")
#define QMC2_ARCADE_PREFIX		QString("Arcade/qmc2-sdlmame/")
#elif defined(QMC2_SDLMESS)
#define QMC2_FRONTEND_PREFIX		QString("Frontend/qmc2-sdlmess/")
#define QMC2_ARCADE_PREFIX		QString("Arcade/qmc2-sdlmess/")
#elif defined(QMC2_MAME)
#define QMC2_FRONTEND_PREFIX		QString("Frontend/qmc2-mame/")
#define QMC2_ARCADE_PREFIX		QString("Arcade/qmc2-mame/")
#elif defined(QMC2_MESS)
#define QMC2_FRONTEND_PREFIX		QString("Frontend/qmc2-mess/")
#define QMC2_ARCADE_PREFIX		QString("Arcade/qmc2-mess/")
#endif

// OS X uses ~/Library/Application Support/app rather than ~/.app
#define QMC2_SYSCONF_PATH		(QString(XSTR(SYSCONFDIR)).replace(QChar(':'), QLatin1String(" ")) + "/qmc2")
#if defined(Q_WS_MAC)
#define QMC2_DOT_PATH			(QDir::homePath() + "/Library/Application Support/qmc2")
#define QMC2_DEFAULT_DATA_PATH		(QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QMC2_DOT_PATH			(QDir::homePath() + "/.qmc2")
#define QMC2_DEFAULT_DATA_PATH		QString("data")
#endif

#endif
