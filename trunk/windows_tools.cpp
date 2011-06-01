#if defined(Q_WS_WIN)

#include "windows_tools.h"
#include <psapi.h>

// To ensure correct resolution of symbols, add psapi.lib to LIBS
// and compile with -DPSAPI_VERSION=1
// (see http://msdn.microsoft.com/en-us/library/ms682623.aspx)

QString winSearchText;
HWND winFoundHandle;

HANDLE winFindProcessHandle(QString procName)
{
	HANDLE processHandle = NULL;
	DWORD procs[QMC2_WIN_MAX_PROCS], bytesNeeded;

	if ( EnumProcesses(procs, sizeof(procs), &bytesNeeded) ) {
		DWORD numProcesses = bytesNeeded / sizeof(DWORD);
		for (int i = 0; i < numProcesses; i++) {
			HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, false, procs[i]);
			if ( hProcess != NULL ) {
				HMODULE hMod;
				DWORD bN;
				if ( EnumProcessModules(hProcess, &hMod, sizeof(hMod), &bN) ) {
					TCHAR processName[MAX_PATH];
					GetModuleBaseName(hProcess, hMod, processName, sizeof(processName)/sizeof(TCHAR));
#ifdef UNICODE
					QString pN = QString::fromUtf16((ushort*)processName);
#else
					QString pN = QString::fromLocal8Bit(processName);
#endif
					if ( pN == procName ) {
						processHandle = hProcess;
						CloseHandle(hProcess);
						break;
					}
				}
				CloseHandle(hProcess);
			}
		}
	}

	return processHandle;
}

BOOL CALLBACK winFindWindowHandleCallbackProc(HWND hwnd, LPARAM lParam)
{
	WCHAR winTitle[QMC2_WIN_MAX_NAMELEN];
	if ( !GetWindow(hwnd, GW_OWNER) ) {
		GetWindowText(hwnd, winTitle, QMC2_WIN_MAX_NAMELEN - 1);
		QString windowTitle = QString::fromWCharArray(winTitle);
		if ( windowTitle == winSearchText )
			winFoundHandle = hwnd;
	}
	return true;
}

HWND winFindWindowHandle(QString windowName)
{
	winFoundHandle = NULL;
	winSearchText = windowName;
	EnumWindows((WNDENUMPROC)winFindWindowHandleCallbackProc, 0);
	return winFoundHandle;
}

HWND winFindWindowHandleOfProcess(Q_PID processInfo)
{
	bool handleFound = false;
	HWND h = GetTopWindow(0);
	while ( h )
	{
		DWORD pid;
		DWORD dwThreadID = GetWindowThreadProcessId(h, &pid);

		if ( pid == processInfo->dwProcessId ) {
			handleFound = true;
			break;
		}

         	h = GetNextWindow(h, GW_HWNDNEXT);
	}
	if ( handleFound )
		return h;
	else
		return NULL;
}

#endif
