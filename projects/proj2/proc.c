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

	sem_t *sem = sem_open (W_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 1);
	if (sem == SEM_FAILED) exit (2);
	
	// Body of process
	
	char typeOfProc;
	FILE *file = NULL;

	(type == 0) ? (typeOfProc = 'A') : (typeOfProc = 'C');

	if (sem_wait (sem) < 0) exit (2);	// Critical section
	file = fopen (FILE_NAME, "at");
	fprintf (file, "%d\t: %c %d\t: started\n", (*sharedMemory)++, typeOfProc, order);
	fclose (file);
	if (sem_post (sem) < 0) exit (2);

	



	sem_close (sem);
	shmdt (sharedMemory);	// Shared memory deteach for dealloc in parent

	exit (0);
}
