#ifndef _MACROS_H_
#define _MACROS_H_

#include <Qt>

#if !defined(QMC2_SVN_REV)
#define QMC2_SVN_REV			0
#endif

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
#define QMC2_EMBED_INDEX		4

// index positions of all game/machine details (upper right)
// (FIXME: also used for image checker tabs)
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
#define QMC2_PCB_INDEX			11
#define QMC2_SOFTWARE_LIST_INDEX	12
#define QMC2_YOUTUBE_INDEX		13

// current format version of the GLC (game list cache)
#define QMC2_GLC_VERSION		3

// index positions of special front end tabs (lower right)
#define QMC2_FRONTENDLOG_INDEX		0
#define QMC2_EMULATORLOG_INDEX		1
#define QMC2_EMULATORCONTROL_INDEX	2
#define QMC2_AUDIOPLAYER_INDEX		3
#define QMC2_DOWNLOADS_INDEX		4

// column to add game icon in game list
#define QMC2_ICON_INDEX			2

// index positions of game/machine list view selector
#define QMC2_VIEW_DETAIL_INDEX		0
#define QMC2_VIEW_TREE_INDEX		1
#define QMC2_VIEW_CATEGORY_INDEX	2
#define QMC2_VIEW_VERSION_INDEX		3

// logical column indizes in game/machine lists
#define QMC2_GAMELIST_COLUMN_GAME	0
#define QMC2_GAMELIST_COLUMN_ICON	1
#define QMC2_GAMELIST_COLUMN_YEAR	2
#define QMC2_GAMELIST_COLUMN_MANU	3
#define QMC2_GAMELIST_COLUMN_NAME	4
#define QMC2_GAMELIST_COLUMN_RTYPES	5
#define QMC2_GAMELIST_COLUMN_PLAYERS	6
#define QMC2_GAMELIST_COLUMN_CATEGORY	7
#define QMC2_GAMELIST_COLUMN_VERSION	8
#define QMC2_MACHINELIST_COLUMN_GAME	QMC2_GAMELIST_COLUMN_GAME
#define QMC2_MACHINELIST_COLUMN_ICON	QMC2_GAMELIST_COLUMN_ICON
#define QMC2_MACHINELIST_COLUMN_YEAR	QMC2_GAMELIST_COLUMN_YEAR
#define QMC2_MACHINELIST_COLUMN_MANU	QMC2_GAMELIST_COLUMN_MANU
#define QMC2_MACHINELIST_COLUMN_NAME	QMC2_GAMELIST_COLUMN_NAME
#define QMC2_MACHINELIST_COLUMN_RTYPES	QMC2_GAMELIST_COLUMN_RTYPES
#define QMC2_MACHINELIST_COLUMN_PLAYERS	QMC2_GAMELIST_COLUMN_PLAYERS

#define QMC2_GAMELIST_COLUMNS		9
#define QMC2_MACHINELIST_COUMNS		7

// logical column indizes in the emulator control panel
#define QMC2_EMUCONTROL_COLUMN_NUMBER	0
#define QMC2_EMUCONTROL_COLUMN_ID	QMC2_EMUCONTROL_COLUMN_NUMBER
#define QMC2_EMUCONTROL_COLUMN_GAME	1
#define QMC2_EMUCONTROL_COLUMN_MACHINE	QMC2_EMUCONTROL_COLUMN_GAME
#define QMC2_EMUCONTROL_COLUMN_STATUS	2
#define QMC2_EMUCONTROL_COLUMN_LED0	3
#define QMC2_EMUCONTROL_COLUMN_LED1	4
#define QMC2_EMUCONTROL_COLUMN_PID	5
#define QMC2_EMUCONTROL_COLUMN_COMMAND	6

// logical column indizes of the MESS device configurator
#define QMC2_DEVCONFIG_COLUMN_NAME	0
#define QMC2_DEVCONFIG_COLUMN_BRIEF	1
#define QMC2_DEVCONFIG_COLUMN_TYPE	2
#define QMC2_DEVCONFIG_COLUMN_TAG	3
#define QMC2_DEVCONFIG_COLUMN_EXT	4
#define QMC2_DEVCONFIG_COLUMN_FILE	5

// page indizes for the software list tool box
#define QMC2_SWLIST_KNOWN_SW_PAGE	0
#define QMC2_SWLIST_FAVORITES_PAGE	1
#define QMC2_SWLIST_SEARCH_PAGE		2

// software snap positions
#define QMC2_SWSNAP_POS_ABOVE_LEFT	0
#define QMC2_SWSNAP_POS_ABOVE_CENTER	1
#define QMC2_SWSNAP_POS_ABOVE_RIGHT	2
#define QMC2_SWSNAP_POS_BELOW_LEFT	3
#define QMC2_SWSNAP_POS_BELOW_CENTER	4
#define QMC2_SWSNAP_POS_BELOW_RIGHT	5
#define QMC2_SWSNAP_POS_DISABLE_SNAPS	6

// logical column indizes of the MESS software lists
#define QMC2_SWLIST_COLUMN_TITLE	0
#define QMC2_SWLIST_COLUMN_NAME		1
#define QMC2_SWLIST_COLUMN_PUBLISHER	2
#define QMC2_SWLIST_COLUMN_YEAR		3
#define QMC2_SWLIST_COLUMN_PART		4
#define QMC2_SWLIST_COLUMN_INTERFACE	5
#define QMC2_SWLIST_COLUMN_LIST		6
#define QMC2_SWLIST_COLUMN_DEVICECFG	7 // used only in 'favorites'

// logical column indizes of the audio effect list
#define QMC2_AUDIOEFFECT_COLUMN_NAME	0
#define QMC2_AUDIOEFFECT_COLUMN_DESC	1
#define QMC2_AUDIOEFFECT_COLUMN_ENABLE	2
#define QMC2_AUDIOEFFECT_COLUMN_SETUP	3

// logical column indizes in emulator options
#define QMC2_EMUOPT_COLUMN_OPTION	0
#define QMC2_EMUOPT_COLUMN_VALUE	1

// logical column indizes in download manager
#define QMC2_DOWNLOAD_COLUMN_STATUS	0
#define QMC2_DOWNLOAD_COLUMN_PROGRESS	1

// logical column indizes in additional/registered emulator list
#define QMC2_ADDTLEMUS_COLUMN_NAME	0
#define QMC2_ADDTLEMUS_COLUMN_EXEC	1
#define QMC2_ADDTLEMUS_COLUMN_WDIR	2
#define QMC2_ADDTLEMUS_COLUMN_ARGS	3

// SDLMAME output notifier FIFO
#define QMC2_SDLMAME_OUTPUT_FIFO	"/tmp/sdlmame_out"
#define QMC2_SDLMESS_OUTPUT_FIFO	QMC2_SDLMAME_OUTPUT_FIFO

// index positions of game list view stack
#define QMC2_VIEWGAMELIST_INDEX		0
#define QMC2_VIEWHIERARCHY_INDEX	1
#define QMC2_VIEWCATEGORY_INDEX		2
#define QMC2_VIEWVERSION_INDEX		3	

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
#define QMC2_OPTIONS_EMU_ADDTLEMUS_INDEX	2

// index positions of sort criteria combobox entries
#define QMC2_SORTCRITERIA_DESCRIPTION		0
#define QMC2_SORTCRITERIA_ROMSTATE		1
#define QMC2_SORTCRITERIA_YEAR			2
#define QMC2_SORTCRITERIA_MANUFACTURER		3
#define QMC2_SORTCRITERIA_GAMENAME		4
#define QMC2_SORTCRITERIA_MACHINENAME		QMC2_SORTCRITERIA_GAMENAME
#define QMC2_SORTCRITERIA_ROMTYPES		5
#define QMC2_SORTCRITERIA_PLAYERS		6
#define QMC2_SORTCRITERIA_CATEGORY		7
#define QMC2_SORTCRITERIA_VERSION		8

// search window timeout in milliseconds
#define QMC2_SEARCH_TIMEOUT		2000

// buffer size for reading zip-files
#define QMC2_ZIP_BUFFER_SIZE		QMC2_32K

// buffer size for reading regular files
#define QMC2_FILE_BUFFER_SIZE		QMC2_64K

// process timeouts in msecs
#define QMC2_PROCESS_POLL_TIME		25
#define QMC2_PROCESS_POLL_TIME_LONG	500
#define QMC2_PROCESS_POLL_RETRIES	30

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

// general polling interval for operations that need to wait until game/machine list reload has finished
#define QMC2_RELOAD_POLL_INTERVAL	250

// time to wait before trying to load a software snapshot (if any) when hovering over the entries of a software list
#define QMC2_SWSNAP_DELAY		25

// time after which the 'forced snapshot' flag is unset automatically
#define QMC2_SWSNAP_UNFORCE_DELAY	100

// gamelist sort criteria
#define QMC2_SORT_BY_DESCRIPTION	0
#define QMC2_SORT_BY_ROM_STATE		1
#define QMC2_SORT_BY_YEAR		2
#define QMC2_SORT_BY_MANUFACTURER	3
#define QMC2_SORT_BY_NAME		4
#define QMC2_SORT_BY_ROMTYPES		5
#define QMC2_SORT_BY_PLAYERS		6
#define QMC2_SORT_BY_CATEGORY		7
#define QMC2_SORT_BY_VERSION		8

// gamelist cursor positioning mode
#define QMC2_CURSOR_POS_VISIBLE		0
#define QMC2_CURSOR_POS_TOP		1
#define QMC2_CURSOR_POS_BOTTOM		2
#define QMC2_CURSOR_POS_CENTER		3

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

// indirectly controls the ROM state filter responsiveness (number of updates)
#define QMC2_STATEFILTER_UPDATES	4

// responsiveness while reading the XML game/machine list cache
#define QMC2_XMLCACHE_RESPONSIVENESS	100

// responsiveness while reading the ROM state cache
#define QMC2_ROMCACHE_RESPONSIVENESS	500

// responsiveness while pre-caching icons
#define QMC2_ICONCACHE_RESPONSIVENESS	250

// responsiveness while loading the YouTube video info map
#define QMC2_YOUTUBE_VIDEO_INFO_RSP	50

// delay in milliseconds before an automatic ROM check gets triggered
#define QMC2_AUTOROMCHECK_DELAY		250

// interval in milliseconds between activity checks
#define QMC2_ACTIVITY_CHECK_INTERVAL	1000

// emulator option types
#define QMC2_EMUOPT_TYPE_UNKNOWN	0
#define QMC2_EMUOPT_TYPE_BOOL		1
#define QMC2_EMUOPT_TYPE_INT		2
#define QMC2_EMUOPT_TYPE_FLOAT		3
#define QMC2_EMUOPT_TYPE_FLOAT1		QMC2_EMUOPT_TYPE_FLOAT
#define QMC2_EMUOPT_TYPE_STRING		4
#define QMC2_EMUOPT_TYPE_FILE		5
#define QMC2_EMUOPT_TYPE_DIRECTORY	6
#define QMC2_EMUOPT_TYPE_COMBO		7
#define QMC2_EMUOPT_TYPE_FLOAT2		8
#define QMC2_EMUOPT_TYPE_FLOAT3		9

// default decimals for float values of emulator options
#define QMC2_EMUOPT_DFLT_DECIMALS	6

// standard sizes
#define QMC2_ONE_KILOBYTE		1024
#define QMC2_ONE_MEGABYTE		1048576
#define QMC2_ONE_GIGABYTE		1073741824
#define QMC2_ONE_TERABYTE		1099511627776
#define QMC2_16K			16*QMC2_ONE_KILOBYTE
#define QMC2_32K			32*QMC2_ONE_KILOBYTE
#define QMC2_64K			64*QMC2_ONE_KILOBYTE
#define QMC2_128K			128*QMC2_ONE_KILOBYTE
#define QMC2_256K			256*QMC2_ONE_KILOBYTE
#define QMC2_512K			512*QMC2_ONE_KILOBYTE

// MiniWebBrowser: hide status bar after how many milliseconds (if no longer required)
#define QMC2_BROWSER_STATUS_TIMEOUT	250

// MiniWebBrowser: in-memory icon cache size
#define QMC2_BROWSER_ICONCACHE_SIZE	QMC2_ONE_MEGABYTE

// item downloader: number of retries on "operation canceled" errors
#define QMC2_DOWNLOAD_OPCANCEL_RETRY	3

// item downloader: base number of milliseconds to wait before automatic retries
// (the real wait time will be between 5 and 10 times QMC2_DOWNLOAD_RETRY_DELAY)
#define QMC2_DOWNLOAD_RETRY_DELAY	1000

// how many milliseconds between download connection checks
#define QMC2_DOWNLOAD_CHECK_TIMEOUT	10000

// how many milliseconds before automatically cleaning up finished downloads
#define QMC2_DOWNLOAD_CLEANUP_DELAY	250

// MAWS homepage URL (default)
#define QMC2_MAWS_HOMEPAGE_URL		"http://maws.mameworld.info/"

// MAWS ROM set base URL (default)
#define QMC2_MAWS_BASE_URL		"http://maws.mameworld.info/maws/romset/%1"

// MAWS image links base URL (default)
#define QMC2_MAWS_IMGLINKS_BASE_URL	"http://maws.mameworld.info/maws/"

// MAWS web cache size (in-memory, 8M fixed for now)
#define QMC2_MAWS_CACHE_SIZE		8 * QMC2_ONE_MEGABYTE

// delay creation of MAWS quick download menu for how many milliseconds
// (use same delay to start auto downloads)
#define QMC2_MAWS_QDL_DELAY		250

// delay forced background widget updates for how many milliseconds
#define QMC2_MAWS_BWU_DELAY		25

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
#define QMC2_USE_PHONON_API		(QMC2_PHONON == 1)

// number of milliseconds to wait before automatically resuming audio/video playback
#define QMC2_AUDIOPLAYER_RESUME_DELAY	2000
#define QMC2_VIDEOPLAYER_RESUME_DELAY	2000

// audio player seek offset (in milliseconds)
#define QMC2_AUDIOPLAYER_SEEK_OFFSET	1000

// audio player fader timeout 
#if defined(QMC2_FADER_SPEED)
#define QMC2_AUDIOPLAYER_FADER_TIMEOUT	QMC2_FADER_SPEED
#else
#define QMC2_AUDIOPLAYER_FADER_TIMEOUT  100
#endif

// audio player fader functions
#define QMC2_AUDIOPLAYER_FADER_PAUSE	0
#define QMC2_AUDIOPLAYER_FADER_PLAY	1

// QMC2 variant launcher specific stuff
#define QMC2_COMMAND_RUNONCE		"runonce"
#define QMC2_VARIANT_SDLMAME_NAME	"qmc2-sdlmame"
#define QMC2_VARIANT_SDLMAME_BUNDLE_ID	"net.arcadehits.qmc2." QMC2_VARIANT_SDLMAME_NAME
#define QMC2_VARIANT_SDLMAME_TITLE	MainWindow::tr("M.A.M.E. Catalog / Launcher II")
#define QMC2_VARIANT_SDLMESS_NAME	"qmc2-sdlmess"
#define QMC2_VARIANT_SDLMESS_BUNDLE_ID	"net.arcadehits.qmc2." QMC2_VARIANT_SDLMESS_NAME
#define QMC2_VARIANT_SDLMESS_TITLE	MainWindow::tr("M.E.S.S. Catalog / Launcher II")
#define QMC2_VARIANT_MAME_NAME		"qmc2-mame.exe"
#define QMC2_VARIANT_MAME_BUNDLE_ID	""
#define QMC2_VARIANT_MAME_TITLE		MainWindow::tr("M.A.M.E. Catalog / Launcher II")
#define QMC2_VARIANT_MESS_NAME		"qmc2-mess.exe"
#define QMC2_VARIANT_MESS_BUNDLE_ID	""
#define QMC2_VARIANT_MESS_TITLE		MainWindow::tr("M.E.S.S. Catalog / Launcher II")

// separation for QMC2 variants
#if defined(QMC2_SDLMAME)
#define QMC2_FRONTEND_PREFIX		QString("Frontend/qmc2-sdlmame/")
#define QMC2_ARCADE_PREFIX		QString("Arcade/qmc2-sdlmame/")
#define QMC2_EMUTYPE_MAME
#elif defined(QMC2_SDLMESS)
#define QMC2_FRONTEND_PREFIX		QString("Frontend/qmc2-sdlmess/")
#define QMC2_ARCADE_PREFIX		QString("Arcade/qmc2-sdlmess/")
#define QMC2_EMUTYPE_MESS
#elif defined(QMC2_MAME)
#define QMC2_FRONTEND_PREFIX		QString("Frontend/qmc2-mame/")
#define QMC2_ARCADE_PREFIX		QString("Arcade/qmc2-mame/")
#define QMC2_EMUTYPE_MAME
#elif defined(QMC2_MESS)
#define QMC2_FRONTEND_PREFIX		QString("Frontend/qmc2-mess/")
#define QMC2_ARCADE_PREFIX		QString("Arcade/qmc2-mess/")
#define QMC2_EMUTYPE_MESS
#endif

// Mac OS X uses "~/Library/Application Support/app" rather than "~/.app"
#define QMC2_SYSCONF_PATH		(QString(XSTR(SYSCONFDIR)).replace(QChar(':'), QLatin1String(" ")) + "/qmc2")
#if defined(Q_WS_MAC)
#define QMC2_DOT_PATH			(QDir::homePath() + "/Library/Application Support/qmc2")
#define QMC2_DEFAULT_DATA_PATH		(QDir::homePath() + "/Library/Application Support/qmc2")
#else
#define QMC2_DOT_PATH			(QDir::homePath() + "/.qmc2")
#define QMC2_DEFAULT_DATA_PATH		QString("data")
#endif

// this allows to change the configuration path dynamically (by adding "-qmc2_config_path <alternate_config_path>" on the command line)
#define QMC2_DYNAMIC_DOT_PATH		(qApp->arguments().indexOf("-qmc2_config_path") >= 0 && qApp->arguments().indexOf("-qmc2_config_path") + 1 <= qApp->arguments().count() ? qApp->arguments()[qApp->arguments().indexOf("-qmc2_config_path") + 1]: QMC2_DOT_PATH)

// determine if memory infomation can be made available at all
#if defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE) && defined(_SC_AVPHYS_PAGES)
#define QMC2_SHOWMEMINFO
#define QMC2_MEMORY_UPDATE_TIME		500
#endif

// X11 only: embedder specific delays (in ms)
#define QMC2_EMBED_DELAY		250
#define QMC2_EMBED_MAXIMIZE_DELAY	100
#define QMC2_EMBED_FOCUS_DELAY		100
#define QMC2_EMBED_PAUSERESUME_DELAY	250

// maximum number of retries to find an emulator window via xwininfo (the emulator may need longer to get ready)
#define QMC2_MAX_XWININFO_RETRIES	5

// responsiveness while loading an additional information source
#if defined(QMC2_EMUTYPE_MESS)
#define QMC2_INFOSOURCE_RESPONSIVENESS	500
#else
#define QMC2_INFOSOURCE_RESPONSIVENESS	5000
#endif

// maximum delay in ms before finally killing an external tool (if required to)
#define QMC2_TOOL_KILL_WAIT		2000

// supported DB drivers
#define QMC2_DB_DRIVER_MYSQL		0
#define QMC2_DB_DRIVER_SQLITE		1

// DB server default ports
#define QMC2_DB_DFLT_PORT_MYSQL		3306

// DB connection check button reset delay
#define QMC2_DB_RESET_CCB_DELAY		1000

// MAME/MESS exit codes (from MAME source -- src/emu/mame.h)
#define	QMC2_MAME_ERROR_NONE		0	// no error
#define	QMC2_MAME_ERROR_FAILED_VALIDITY	1	// failed validity checks
#define	QMC2_MAME_ERROR_MISSING_FILES	2	// missing files
#define	QMC2_MAME_ERROR_FATALERROR	3	// some other fatal error
#define	QMC2_MAME_ERROR_DEVICE		4	// device initialization error (MESS-specific)
#define	QMC2_MAME_ERROR_NO_SUCH_GAME	5	// game was specified but doesn't exist
#define	QMC2_MAME_ERROR_INVALID_CONFIG	6	// some sort of error in configuration
#define	QMC2_MAME_ERROR_IDENT_NONROMS	7	// identified all non-ROM files
#define	QMC2_MAME_ERROR_IDENT_PARTIAL	8	// identified some files but not all
#define	QMC2_MAME_ERROR_IDENT_NONE	9	// identified no files
#define	QMC2_MAME_ERROR_UNKNOWN		-1	// unknown exit code

// exchangable (de)compression routines
#define QMC2_COMPRESS(data)		qCompress((data))
#define QMC2_UNCOMPRESS(data)		(data).isEmpty() ? QByteArray() : qUncompress(data)

// timeout (in ms) for locking the log-mutex
#define QMC2_LOG_MUTEX_LOCK_TIMEOUT	100

// X11 only: time (in ms) between KeyPress and KeyRelease events when simulating keys sent to an emulator
#define QMC2_XKEYEVENT_TRANSITION_TIME	50

#endif
