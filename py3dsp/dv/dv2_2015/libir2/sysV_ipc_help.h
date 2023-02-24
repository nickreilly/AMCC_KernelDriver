
/**********************************************************************************
** sysV_ipc_help.h - helper fuction for systemV IPC (msg queue, semaphones, shared 
**    memory). Later IRTF code used the poxis IPC.
**
***********************************************************************************
*/

int msgtran( int key, int create_it);
int shmtran( int key, int nbytes, int create_it);
int semtran( int key, int create_it);
int sem_op( int sid, int value );

