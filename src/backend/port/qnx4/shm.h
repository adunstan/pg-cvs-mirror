/*-------------------------------------------------------------------------
 *
 * shm.h
 *	  System V Shared Memory Emulation
 *
 * Copyright (c) 1999, repas AEG Automation GmbH
 *
 *
 * IDENTIFICATION
 *	  $Header: /home/cvsmirror/pg/pgsql/src/backend/port/qnx4/Attic/shm.h,v 1.4 2001/10/25 05:49:40 momjian Exp $
 *
 *-------------------------------------------------------------------------
 */

#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#include <sys/ipc.h>

#ifdef	__cplusplus
extern		"C"
{
#endif

#define SHM_R	0400			/* read permission */
#define SHM_W	0200			/* write permission */

	struct shmid_ds
	{
		int			dummy;
		int			shm_nattch;
	};

	extern void *shmat(int shmid, const void *shmaddr, int shmflg);
	extern int	shmdt(const void *addr);
	extern int	shmctl(int shmid, int cmd, struct shmid_ds * buf);
	extern int	shmget(key_t key, size_t size, int flags);

#ifdef	__cplusplus
}
#endif
#endif	 /* _SYS_SHM_H */
