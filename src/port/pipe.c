/*-------------------------------------------------------------------------
 *
 * pipe.c
 *	  pipe()
 *
 * Copyright (c) 1996-2003, PostgreSQL Global Development Group
 *
 *	This is a replacement version of pipe for Win32 which allows
 *	returned handles to be used in select(). Note that read/write calls
 *	must be replaced with recv/send.
 *
 * IDENTIFICATION
 *	  $PostgreSQL$
 *
 *-------------------------------------------------------------------------
 */

#include "postgres.h"

int
pgpipe(int handles[2])
{
	SOCKET		s;
	struct sockaddr_in serv_addr;
	int			len = sizeof(serv_addr);

	handles[0] = handles[1] = INVALID_SOCKET;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
		return -1;

	memset((void *) &serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(0);
	serv_addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(s, (SOCKADDR *) & serv_addr, len) == SOCKET_ERROR ||
		listen(s, 1) == SOCKET_ERROR ||
		getsockname(s, (SOCKADDR *) & serv_addr, &len) == SOCKET_ERROR ||
		(handles[1] = socket(PF_INET, SOCK_STREAM, 0)) == INVALID_SOCKET)
	{
		closesocket(s);
		return -1;
	}

	if (connect(handles[1], (SOCKADDR *) & serv_addr, len) == SOCKET_ERROR ||
		(handles[0] = accept(s, (SOCKADDR *) & serv_addr, &len)) == INVALID_SOCKET)
	{
		closesocket(handles[1]);
		handles[1] = INVALID_SOCKET;
		closesocket(s);
		return -1;
	}
	closesocket(s);
	return 0;
}
