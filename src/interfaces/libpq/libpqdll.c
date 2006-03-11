/* $PostgreSQL: pgsql/src/backend/utils/misc/guc.c,v 1.314 2006/03/07 02:54:23 momjian Exp $ */

#define WIN32_LEAN_AND_MEAN
#include <winsock.h>
#include <windows.h>
#include "win32.h"


BOOL		WINAPI
DllMain(HINSTANCE hinstDLL, DWORD fdwReason,
		LPVOID lpReserved)
{
	WSADATA		wsaData;

	switch (fdwReason)
	{
		case DLL_PROCESS_ATTACH:
			if (WSAStartup(MAKEWORD(1, 1), &wsaData))
			{
				/*
				 * No really good way to do error handling here, since we
				 * don't know how we were loaded
				 */
				return FALSE;
			}
			break;
		case DLL_PROCESS_DETACH:
			WSACleanup();
			break;
	}

	return TRUE;
}
