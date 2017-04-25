/**
*   Author: Tomas Bruckner
*   Date: 4.4.2015    
*   Description: 
*       Advanced example of two processes sharing variables in C for Linux.
*       make && ./example_advanced
*
*       If you are getting SIGSEGV, try:
*           rm /dev/shm/sem.example_advanced
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <time.h>
#include <signal.h>

void set_resources();
void do_stuff();
void clean_resources();

sem_t *semaphore;
int *sharedCounter = NULL, *notSharedCounter = NULL, sharedCounterId = 0;

int main() 
{
    srandom(time(0));   // initialize random number generator
    setbuf(stdout, NULL);  // for valid printing

    set_resources();

    int processId = 0;
    pid_t processPid = fork();

    if (processPid == 0) // child
    {
        for(int i = 0; i < 5; i++)
        {
            int randomTime = random() % 1000 * 1000;

            do_stuff("Chld");

            usleep(randomTime);
        }

        exit(0);
    }
    else if (processPid > 0) //parent
    {  
        processId = processPid;
        for(int i = 0; i < 5; i++)
        {
            int randomTime = random() % 500 * 1000;
        
            do_stuff("Prnt");
            
            usleep(randomTime);  
        }
    } 
    else  // error
    {
        // handle error
    }

    waitpid(processId, NULL, 0);

    clean_resources();

    return 0;
}

void set_resources()
{
    if ((sharedCounterId = shmget(IPC_PRIVATE, sizeof (int), IPC_CREAT | 0666)) == -1) 
    { 
        // handle error
    }
    
    if ((sharedCounter = (int *) shmat(sharedCounterId, NULL, 0)) == NULL) 
    { 
        // handle error
    }
 
    if ((notSharedCounter = (int *) malloc(sizeof(int))) == NULL)
    {
        // handle error
    }


    *sharedCounter = 1;
    *notSharedCounter = 1;
    
    if ((semaphore = sem_open("/example_advanced", O_CREAT | O_EXCL, 0666, 1)) == SEM_FAILED) 
    { 
        // handle error
    }
}

void do_stuff(char * processName)
{
    sem_wait(semaphore);
    printf("%s not shared: %d shared: %d\n", processName, (*notSharedCounter)++, (*sharedCounter)++);
    sem_post(semaphore);
}

void clean_resources()
{
    free(notSharedCounter);
    shmctl(sharedCounterId, IPC_RMID, NULL);
    sem_close(semaphore);
    sem_unlink("/example_advanced");
}
