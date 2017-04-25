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

//int semid;
int *shm;

sem_t *sem_mutex;
sem_t *sem_space;
sem_t *sem_items;

void sol_init() {
    int shmID;

    // kontrola uspesnosti !!!
    shmID = shm_open(shmKEY, O_RDWR, S_IRUSR | S_IWUSR);
    shm = (int*)mmap(NULL, shmSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, shmID, 0);
    close(shmID);

    sem_mutex = sem_open(semMUTEX, O_RDWR);
    sem_space = sem_open(semSPACE, O_RDWR);
    sem_items = sem_open(semITEMS, O_RDWR);
}

void sol_destroy() {
    munmap(shm, shmSIZE);
    sem_close(sem_mutex);
    sem_close(sem_space);
    sem_close(sem_items);
}

void prod() {

    sol_init();

    for (int i = 1; i <= 5; i++) {
        sem_wait(sem_space);
        sem_wait(sem_mutex);
        *shm = i*10;
        fprintf(logFile, "producent: %d\n", *shm);
        fprintf(stdout, "producent: %d\n", *shm);
        fflush(logFile);
        sem_post(sem_mutex);
        sem_post(sem_items);
    }

    sol_destroy();
    exit(0);
}

void cons() {

    sol_init();

    for (int i = 1; i <= 5; i++) {
        sem_wait(sem_items);
        sem_wait(sem_mutex);
        int v = *shm;
        fprintf(logFile, "konzument: %d\n", v);
        fprintf(stdout, "konzument: %d\n", v);
        fflush(logFile);
        sem_post(sem_mutex);
        sem_post(sem_space);
    }

    sol_destroy();

    exit(0);
}

