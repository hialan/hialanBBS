#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/sem.h>

/* ----------------------------------------------------- */
/* semaphore : for critical section                      */
/* ----------------------------------------------------- */

#define SEM_FLG        0600    /* semaphore mode */


void              /*  sem_init(BSEM_KEY,&ap_semid)  */
sem_init(int semkey,int *semid)
{
#if defined(SunOS) || defined(SOLARIS) || defined(LINUX)
  int semval = 1;
#else  
  union semun semval;  // 解決某些 OS 會把 1 認為是 int 之問題
  semval.val = 1;
#endif  

  *semid = semget(semkey, 1, 0);
  if (*semid == -1)
  {
    *semid = semget(semkey, 1, IPC_CREAT | SEM_FLG);
//    if (*semid == -1)
//      attach_err(semkey, "semget");
    semctl(*semid, 0, SETVAL, semval);
  }
}

void
sem_lock(int op,int semid)   /*  sem_lock(SEM_LEAVE,ap_semid)  */
{
  struct sembuf sops;

  sops.sem_num = 0;
  sops.sem_flg = SEM_UNDO;
  sops.sem_op = op;
  semop(semid, &sops, 1);
}
