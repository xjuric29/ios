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

void printError (int type) {
	switch (type) {
		case 1:
			fprintf (stderr, "Usage:\n"
				"./proj2 A C AGT CGT AWT CWT\n");
			exit (type);
		case 2:
			fprintf (stderr, "System call error\n");
			exit (type);
	}
}

int main (int argc, char **argv) {
	
	// Arguments parsing
	
	processControl mainProc;
	const int numberOfArgs = 7;
	
	if (argc != numberOfArgs || 
	    sscanf (argv[1], "%d", &mainProc.a) != 1 || mainProc.a <= 0 || 
	    sscanf (argv[2], "%d", &mainProc.c) != 1 || mainProc.c <= 0 ||
	    sscanf (argv[3], "%d", &mainProc.agt) != 1 || mainProc.agt < 0 || mainProc.agt > 5000 ||
            sscanf (argv[4], "%d", &mainProc.cgt) != 1 || mainProc.cgt < 0 || mainProc.cgt > 5000 ||
	    sscanf (argv[5], "%d", &mainProc.awt) != 1 || mainProc.awt < 0 || mainProc.awt > 5000 ||
            sscanf (argv[6], "%d", &mainProc.cwt) != 1 || mainProc.cwt < 0 || mainProc.cwt > 5000) {
		printError (1);
	}
	mainProc.aAct = 0;
	mainProc.cAct = 0;

	// Shared memory

	int segmentID, *sharedMemory;

	segmentID = shmget (IPC_PRIVATE, sizeof (int), IPC_CREAT | IPC_EXCL | S_IRUSR | S_IWUSR);
	if (segmentID < 0) printError (2);
	sharedMemory = shmat (segmentID, NULL, 0);
	*sharedMemory = 1;	// Number of first line
	shmdt (sharedMemory);

	// File reset

	fclose (fopen (FILE_NAME, "wt"));

	// Creating processes
	
	pid_t pidGenerateAdult, pidGenerateChild;
	
	printf ("%d\n", IPC_PRIVATE);
	if ((pidGenerateAdult = fork ()) == 0) {	// Generate adults process
		//sleep (10);
		newProc (0, 1, 0, segmentID);
	}
	
	// Back in parrent
	else if (pidGenerateAdult < 0) printError (2);
	
	if ((pidGenerateChild = fork ()) == 0) {        // Generate childs process
        	//sleep (20);
		newProc (1, 1, 0, segmentID);
	}

        // Back in parent
        else if (pidGenerateAdult < 0) {        // fork se nezdari
                wait (NULL);	
		printError (2);
        }
	
	printf ("Som rodic, pid: %d\n", getpid ());
	

	while (wait (NULL) != -1);
	sem_unlink (W_SEMAPHORE);	// Cleaning own semaphore
	shmctl (segmentID, IPC_RMID, NULL);

	exit (0);
}
