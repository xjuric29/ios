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

void newProc (const int type, const int order, const int waitTime, int segmentID) {	// Type: 0 - adult, 1 -child
	
	// Shared memory
	
	int *sharedMemory = NULL;

	sharedMemory = shmat (segmentID, NULL, 0);
	if (sharedMemory == NULL) exit (2);

	// Semaphore

	sem_t *semM = sem_open (M_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 1); 
	sem_t *semA = sem_open (A_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 0);
	sem_t *semC = sem_open (C_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 0);
	
	if (semM == SEM_FAILED || semA == SEM_FAILED || semC == SEM_FAILED) exit (2);
	
	// Body of process
	
	char typeOfProc;
	FILE *file = NULL;
	int tmp;

	(type == 0) ? (typeOfProc = 'A') : (typeOfProc = 'C');		// Set type in char of process

	sem_wait (semM);	// Start
	file = fopen (FILE_NAME, "at");
	setbuf (file, NULL);
	fprintf (file, "%d\t: %c %d\t: started\n", sharedMemory[COUNT]++, typeOfProc, order);
	fflush (file);
	fclose (file);
	sem_post (semM);

	if (type == 0) {	// Adult block
		sem_wait (semM);
		sharedMemory[ADULTS]++;
		if (sharedMemory[WAITING]) {
			(sharedMemory[WAITING] < 3) ? (tmp = sharedMemory[WAITING]) : (tmp = 3);
			for (int i = 0; i < tmp; i++) sem_post (semC);
			sharedMemory[WAITING] -= tmp;
			sharedMemory[CHILDREN] += tmp;		
		}
		file = fopen (FILE_NAME, "at");
        	setbuf (file, NULL);
		fprintf (file, "%d\t: %c %d\t: enter\n", sharedMemory[COUNT]++, typeOfProc, order);
		fflush (file);
        	fclose (file);
		sem_post (semM);
		
		if (waitTime != 0) usleep (rand () % (waitTime + 1));		

		sem_wait (semM);
		file = fopen (FILE_NAME, "at");
		setbuf (file, NULL);
        	fprintf (file, "%d\t: %c %d\t: trying to leave\n", sharedMemory[COUNT]++, typeOfProc, order);
		if (sharedMemory[CHILDREN] <= (3 * (sharedMemory[ADULTS] - 1))) {
			sharedMemory[ADULTS]--;
			sharedMemory[ADULTS_REM]--;
			fprintf (file, "%d\t: %c %d\t: leave\n", sharedMemory[COUNT]++, typeOfProc, order);
			fflush (file);
			fclose (file);
			sem_post (semM);
		}
		else {
			sharedMemory[LEAVING]++;
			fprintf (file, "%d\t: %c %d\t: waiting : %d : %d\n", sharedMemory[COUNT]++, typeOfProc, order, sharedMemory[ADULTS], sharedMemory[CHILDREN]);
                        fflush (file);
			fclose (file);
			sem_post (semM);
			sem_wait (semA);
			sem_wait (semM);
			file = fopen (FILE_NAME, "at");
			setbuf (file, NULL);
			fprintf (file, "%d\t: %c %d\t: leave\n", sharedMemory[COUNT]++, typeOfProc, order);
			fflush (file);
			fclose (file);
			sem_post (semM);
		}

	}
	else {	// Child block
		sem_wait (semM);
		file = fopen (FILE_NAME, "at");
		setbuf (file, NULL);
                if (sharedMemory[CHILDREN] < (3 * sharedMemory[ADULTS]) || sharedMemory[ADULTS_REM] == 0) {
			sharedMemory[CHILDREN]++;
                	fprintf (file, "%d\t: %c %d\t: enter\n", sharedMemory[COUNT]++, typeOfProc, order);
                	fflush (file);
			fclose (file);
			sem_post (semM);
                }
                else {
                        sharedMemory[WAITING]++;
                        fprintf (file, "%d\t: %c %d\t: waiting : %d : %d\n", sharedMemory[COUNT]++, typeOfProc, order, sharedMemory[ADULTS], sharedMemory[CHILDREN]);
			fflush (file);
			fclose (file);
			sem_post (semM);
                        sem_wait (semC);
                	sem_wait (semM);
                        file = fopen (FILE_NAME, "at");
			setbuf (file, NULL);
                        fprintf (file, "%d\t: %c %d\t: enter\n", sharedMemory[COUNT]++, typeOfProc, order);     
                        fflush (file);
			fclose (file);
			sem_post (semM);
		}
		
		if (waitTime != 0) usleep (rand () % (waitTime + 1));	
	
                sem_wait (semM);
                sharedMemory[CHILDREN]--;
                if (sharedMemory[LEAVING] && (sharedMemory[CHILDREN] <= (3 * (sharedMemory[ADULTS] - 1)))) {
                        sharedMemory[LEAVING]--;
                        sharedMemory[ADULTS]--;
			sharedMemory[ADULTS_REM]--;
                        sem_post (semA);
                }
		file = fopen (FILE_NAME, "at");
		setbuf (file, NULL);
                fprintf (file, "%d\t: %c %d\t: trying to leave\n", sharedMemory[COUNT]++, typeOfProc, order);
		fprintf (file, "%d\t: %c %d\t: leave\n", sharedMemory[COUNT]++, typeOfProc, order);
		fflush (file);
		fclose (file);
		sem_post (semM);
	}

	sem_close (semM);
	sem_close (semA);
	sem_close (semC);
	shmdt (sharedMemory);	// Shared memory deteach for dealloc in parent

	exit (0);
}
