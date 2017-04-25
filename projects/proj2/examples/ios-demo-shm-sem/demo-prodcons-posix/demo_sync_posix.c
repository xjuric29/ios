#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <unistd.h>
#include <sys/sem.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <semaphore.h>

// -------------------
#include "common.h"
#include "solution.h"

pid_t consPID;
pid_t prodPID;

void destroy();

void init() {
    sem_t *semid;
    int shmID;
    int *shm;

    // vytvoreni sdilene pameti - opet overit uspesnost ...
    // inicializace ...
    shmID = shm_open(shmKEY, O_CREAT | O_EXCL | O_RDWR, S_IRUSR | S_IWUSR);
    ftruncate(shmID, shmSIZE);
    shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    *shm = 0;
    munmap(shm, shmSIZE);
    close(shmID);

    // vytvorime a inicializujeme semafory
    semid = sem_open(semMUTEX, O_CREAT, 0666, 1);
    sem_close(semid);
    semid = sem_open(semITEMS, O_CREAT, 0666, 0);
    sem_close(semid);
    semid = sem_open(semSPACE, O_CREAT, 0666, BUFSIZE);
    sem_close(semid);

    // vytvoreni souboru pro zapis
    logFile = fopen(FNAME, "w");
    if (logFile == NULL) { /* ... */ }
}

// Releases resources.
void destroy() {

    sem_unlink(semMUTEX);
    sem_unlink(semITEMS);
    sem_unlink(semSPACE);
    shm_unlink(shmKEY);

    fclose(logFile);
}


//int main(int argc, char *argv[])
int main(void)
{
    int pid;
    setbuf(stdout,NULL);
    setbuf(stderr,NULL);

    init();

    // systemove volani - vzdy je vhodne overit uspesnost!
    if ((pid = fork()) < 0) {
        perror("fork"); 
        exit(2);
    }

    if (pid == 0) { // child
        cons();
        exit(0);
    } else { // parent.
        consPID = pid;
        //--
        pid = fork();
        //--
        if (pid == 0) { // child
            prod();
            exit(0);
        } else { // parent
            prodPID = pid;
        }
    }

    // pockame az vsichni skonci
    waitpid(consPID, NULL, 0);
    waitpid(prodPID, NULL, 0);

    // zrusime zdroje
    destroy();

    return 0;
}
