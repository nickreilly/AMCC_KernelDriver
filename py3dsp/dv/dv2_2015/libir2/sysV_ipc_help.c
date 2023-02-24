/**********************************************************************************
** sysV_ipc_help.c - helper fuction for systemV IPC (msg queue, semaphones, shared 
**    memory). Later IRTF code used the poxis IPC.
**
***********************************************************************************
*/

#define EXTERN extern

/*--------------------------
 *  Standard include files
 *--------------------------
 */

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/time.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

#include "sysV_ipc_help.h"

/****************************************************************************
**  Messages Queues
*****************************************************************************
**/

/*-------------------------------------------------------------------------
**   msgtran() - Translate message queue key to ID.
**        If create_it is true, it will create a new message queue.
**        if a 'key' already exist, it is destroy and re-created.
**        returns:
**                       >0 - queue ID.
**                       -1 - Error.
**-------------------------------------------------------------------------
*/
int msgtran( key, create_it )
   int key;                      /* 'Name' of message queue */
   int create_it;                /* Create a new message queue */
{
   int msqid;
   struct msqid_ds sbuf;

   msqid = msgget( (key_t) key, 0666);
   if( !create_it )
      return msqid;

   /* if msg queue already exist, destroy it.  */
   if( msqid != -1)
   {
      /*  Destroy Message queue */
      if( msgctl(msqid, IPC_RMID, &sbuf) < 0)
         return -1;
   }

   /* Create message queue with "name" key  */
   if( (msqid = msgget( (key_t) key, IPC_CREAT | 0666)) < 0)
         return -1;

   return msqid;
}


/****************************************************************************
**  Semaphore 
*****************************************************************************
**/

/* The following is from the linux man page on semctl() */
#if defined(__GNU_LIBRARY__) && !defined(_SEM_SEMUN_UNDEFINED)
	/* union semun is defined by including <sys/sem.h> */
#else
	/* according to X/OPEN we have to define it ourselves */
	union semun {
			int val;                    /* value for SETVAL */
			struct semid_ds *buf;       /* buffer for IPC_STAT, IPC_SET */
			unsigned short int *array;  /* array for GETALL, SETALL */
			struct seminfo *__buf;      /* buffer for IPC_INFO */
	};
#endif

/*-------------------------------------------------------------------------
**   semtran() - translate semaphore key to ID.
**        If create_it is true, it will create a new semaphore.
**        if a 'key' already exist, it is destroy and re-created.
**        returns:
**                  >0 - semaphore ID.
**                  -1 - Error.
**-------------------------------------------------------------------------
*/
int semtran( key, create_it )
   int   key;                      /* 'Name' of message queue */
   int   create_it;
{
   int sid;
   int rc;
#if defined(__linux__)
	union semun sem_arg;
#else
   void * sem_arg = NULL;
#endif

   sid = semget( (key_t) key, 1, 0666);
   if( !create_it )
      return sid;

   /* if semaphore already exist, destroy it.  */
   if( sid != -1)
   {
      /*  Destroy semaphore */
		if( (rc=semctl(sid, 0, IPC_RMID, sem_arg)) < 0)
      {
			perror("semctl");
         return -1;
      }
   }

   /* Create semaphore with "name" key  */
   if( (sid = semget( (key_t) key, 1, IPC_CREAT | 0666)) < 0)
         return -1;

   return sid;
}

/*-------------------------------------------------------------------------
**  sem_op() - calls semop() to increment or decrement by value.
**  You can define these in you application if you used these these function:
**     #define P( a )          sem_op( a, -1)
**     #define V( a )          sem_op( a,  1)
**-------------------------------------------------------------------------
*/
int sem_op( int sid, int value )
{
   struct sembuf sb;

   sb.sem_num = 0;
   sb.sem_op = value;
   sb.sem_flg = 0;
   if( semop(sid, &sb, 1) == -1)
	{
      perror("semop");
		return -1;
	}
	return 0;
}

/****************************************************************************
**  Help with shared memory.
*****************************************************************************
**/

/*-------------------------------------------------------------------------
**   shmtran() - translate shared memroy key to segment ID.
**        If create_it is true, it will create a new segment ID.
**        if a 'key' already exist, it is destroy and re-created.
**        returns:
**                  >=0 - segment ID.
**                   -1 - Error.
**-------------------------------------------------------------------------
*/
int shmtran( key, nbytes, create_it )
   int   key;                 /* Shared memory key             */
   int   nbytes;              /* Size of shared memory segment */
   int   create_it;
{
   int segid;

   segid = shmget( (key_t) key, nbytes, 0666);
   if( !create_it )
      return segid;

   /* if segment ID already exist, destroy it.  */
   if( segid != -1)
   {
      if( shmctl(segid, IPC_RMID, NULL) < 0)
      {
         perror("shmtran()-shmctl");
         return -1;
      }
   }

   /* Create segment ID with "name" key  */
   if( (segid = shmget( (key_t) key, nbytes, IPC_CREAT | 0666)) < 0)
      {
         perror("shmtran()-shmget");
         return -1;
      }

   return segid;
}



