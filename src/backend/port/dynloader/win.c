/* $Header: /home/cvsmirror/pg/pgsql/src/backend/port/dynloader/Attic/win.c,v 1.2 2003/03/21 17:18:34 petere Exp $ */

#include <windows.h>

char *
dlerror(void)
{
	return "error";
}

int
dlclose(void *handle)
{
	return FreeLibrary((HMODULE)handle) ? 0 : 1;
}

void *
dlsym(void *handle, const char *symbol)
{
	return (void *)GetProcAddress((HMODULE)handle, symbol);
}

void *
dlopen(const char *path, int mode)
{
	return (void *)LoadLibrary(path);
}
