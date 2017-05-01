#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <sys/stat.h>
#include <fcntl.h>      
#include <semaphore.h>

#include "proc.h"

typedef struct processControl processControl;

void cacthSignalGenProc (int signal) {
	
	// Shared memory
	
	processControl *sharedMemory = NULL;
	int segmentID = shmget (ftok (PATH, 1), sizeof (processControl), IPC_EXCL | S_IRUSR | S_IWUSR);
	
	sharedMemory = shmat (segmentID, NULL, 0); 
	//if (sharedMemory == NULL);	// error

	switch (signal) {
		case SIGUSR1:
			if (sharedMemory->adultsRem == 0 && sharedMemory->childrenRem == 0 ) {
				sem_t *semE = sem_open (E_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 0);
				for (int i = 0; i < (sharedMemory->a + sharedMemory->c); i++) {
					sem_post (semE);
				}
				sem_close (semE);
			}
			break;
	}
	shmdt (sharedMemory);
	return;
}

void newProc (const int type, const int order, int segmentID) {	// Type: 0 - adult, 1 -child
	
	// Shared memory
	
	processControl *sharedMemory = NULL;

	sharedMemory = shmat (segmentID, NULL, 0);
	if (sharedMemory == NULL) exit (2);

	// Semaphore

	sem_t *semM = sem_open (M_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 1); 
	sem_t *semA = sem_open (A_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 0);
	sem_t *semC = sem_open (C_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 0);
	sem_t *semE = sem_open (E_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 0);
	
	if (semM == SEM_FAILED || semA == SEM_FAILED || semC == SEM_FAILED || semC == SEM_FAILED) exit (2);
	
	// Body of process
	
	char typeOfProc;
	FILE *file = NULL;
	int tmp;

	(type == 0) ? (typeOfProc = 'A') : (typeOfProc = 'C');		// Set type in char of process

	sem_wait (semM);	// Start
	file = fopen (FILE_NAME, "at");
	setbuf (file, NULL);
	fprintf (file, "%d\t: %c %d\t: started\n", sharedMemory->count++, typeOfProc, order);
	fflush (file);
	fclose (file);
	sem_post (semM);

	if (type == 0) {	// Adult block
		sem_wait (semM);
		sharedMemory->adults++;
		if (sharedMemory->waiting) {
			(sharedMemory->waiting < 3) ? (tmp = sharedMemory->waiting) : (tmp = 3);
			for (int i = 0; i < tmp; i++) sem_post (semC);
			sharedMemory->waiting -= tmp;
			sharedMemory->children += tmp;		
		}
		file = fopen (FILE_NAME, "at");
        	setbuf (file, NULL);
		fprintf (file, "%d\t: %c %d\t: enter\n", sharedMemory->count++, typeOfProc, order);
		fflush (file);
        	fclose (file);
		sem_post (semM);
		
		if (sharedMemory->awt != 0) usleep (rand () % (sharedMemory->awt + 1));		

		sem_wait (semM);
		file = fopen (FILE_NAME, "at");
		setbuf (file, NULL);
        	fprintf (file, "%d\t: %c %d\t: trying to leave\n", sharedMemory->count++, typeOfProc, order);
		if (sharedMemory->children <= (3 * (sharedMemory->adults - 1))) {
			sharedMemory->adults--;
			sharedMemory->adultsRem--;
			fprintf (file, "%d\t: %c %d\t: leave\n", sharedMemory->count++, typeOfProc, order);
			fflush (file);
			fclose (file);
			sem_post (semM);
		}
		else {
			sharedMemory->leaving++;
			fprintf (file, "%d\t: %c %d\t: waiting : %d : %d\n", sharedMemory->count++, typeOfProc, order, sharedMemory->adults, sharedMemory->children);
                        fflush (file);
			fclose (file);
			sem_post (semM);
			sem_wait (semA);
			sem_wait (semM);
			file = fopen (FILE_NAME, "at");
			setbuf (file, NULL);
			fprintf (file, "%d\t: %c %d\t: leave\n", sharedMemory->count++, typeOfProc, order);
			fflush (file);
			fclose (file);
			sem_post (semM);
		}

	}
	else {	// Child block
		sem_wait (semM);
		file = fopen (FILE_NAME, "at");
		setbuf (file, NULL);
                if (sharedMemory->children < (3 * sharedMemory->adults) || sharedMemory->adultsRem == 0) {
			sharedMemory->children++;
                	fprintf (file, "%d\t: %c %d\t: enter\n", sharedMemory->count++, typeOfProc, order);
                	fflush (file);
			fclose (file);
			sem_post (semM);
                }
                else {
                        sharedMemory->waiting++;
                        fprintf (file, "%d\t: %c %d\t: waiting : %d : %d\n", sharedMemory->count++, typeOfProc, order, sharedMemory->adults, sharedMemory->children);
			fflush (file);
			fclose (file);
			sem_post (semM);
                        sem_wait (semC);
                	sem_wait (semM);
                        file = fopen (FILE_NAME, "at");
			setbuf (file, NULL);
                        fprintf (file, "%d\t: %c %d\t: enter\n", sharedMemory->count++, typeOfProc, order);     
                        fflush (file);
			fclose (file);
			sem_post (semM);
		}
		
		if (sharedMemory->cwt != 0) usleep (rand () % (sharedMemory->cwt + 1));	
			
                sem_wait (semM);
                sharedMemory->children--;
		sharedMemory->childrenRem--;
                if (sharedMemory->leaving && (sharedMemory->children <= (3 * (sharedMemory->adults - 1)))) {
                        sharedMemory->leaving--;
                        sharedMemory->adults--;
			sharedMemory->adultsRem--;
                        sem_post (semA);
                }
		file = fopen (FILE_NAME, "at");
		setbuf (file, NULL);
                fprintf (file, "%d\t: %c %d\t: trying to leave\n", sharedMemory->count++, typeOfProc, order);
		fprintf (file, "%d\t: %c %d\t: leave\n", sharedMemory->count++, typeOfProc, order);
		fflush (file);
		fclose (file);
		sem_post (semM);
	}
	kill (getppid (), SIGUSR1);
	sem_wait (semE);	// Waiting while all child/adult process ends
	sem_wait (semM);        // End
        file = fopen (FILE_NAME, "at");
        setbuf (file, NULL);
        fprintf (file, "%d\t: %c %d\t: finished\n", sharedMemory->count++, typeOfProc, order);
        fflush (file);
        fclose (file);
        sem_post (semM);


	sem_close (semM);
	sem_close (semA);
	sem_close (semC);
	sem_close (semE);
	shmdt (sharedMemory);	// Shared memory deteach for dealloc in parent

	exit (0);
}
