/**
*   Author: Tomas Bruckner
*   Date: 4.4.2015    
*   Description: 
*       Basic example of two processes synchronized by two semaphores in C for Linux.
*       make && ./example_basic
*
*       If you are getting SIGSEGV, try:
*           rm /dev/shm/sem.example_basic1 /dev/shm/sem.example_basic2
**/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/shm.h>
#include <signal.h>

sem_t *semaphore1, *semaphore2;
int counter = 1;

int main() 
{
    setbuf(stdout, NULL);  // set buffer

    // create two locked semaphores
    if((semaphore1 = sem_open("/example_basic1", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
    	// handle error
    }

    if((semaphore2 = sem_open("/example_basic2", O_CREAT | O_EXCL, 0666, 0)) == SEM_FAILED)
    {
    	// handle error
    }

    int processId = 0;
    pid_t processPid = fork();

    if (processPid == 0) // child
    {
        for(int i = 0; i < 5; i++)
        {
            sem_wait(semaphore1);
            printf("Chld process %d\n", counter++);
            sem_post(semaphore2);
        }
            
        sem_wait(semaphore1);
        printf("Chld ended\n");
        exit(0);
    }
    else if (processPid > 0) //parent
    {  
        processId = processPid;
     
        for(int i = 0; i < 5; i++)
        {
            sem_post(semaphore1);
            sem_wait(semaphore2);
            printf("Prnt process %d\n", counter++);
        }
    } 
    else  // error
    {
        // handle error
    }

    printf("Waiting for chld to end\n");
    sem_post(semaphore1);

    // wait for parent process to end
    waitpid(processId, NULL, 0);

    printf("Prnt ended\n");
    // close semaphores
    sem_close(semaphore1);
    sem_close(semaphore2);

    // remove semaphores
    sem_unlink("/example_basic1");
    sem_unlink("/example_basic2");

    return 0;
}
