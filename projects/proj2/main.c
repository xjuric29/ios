#include <stdio.h>
#include <stdlib.h>
#include <string.h>
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
	
	// Shared memory

	int segmentID;
	processControl *sharedMemory;

	segmentID = shmget (ftok (PATH, 1), sizeof (processControl), IPC_CREAT | S_IRUSR | S_IWUSR);	// See proc.h for more details about shared structure
	
	if (segmentID < 0) printError (2);
	sharedMemory = shmat (segmentID, NULL, 0);
	memset (sharedMemory, 0, sizeof (processControl));
	sharedMemory->count = 1;

	// Arguments parsing
	
        const int numberOfArgs = 7;

        if (argc != numberOfArgs ||
            sscanf (argv[1], "%d", &(sharedMemory->a)) != 1 || sharedMemory->a <= 0 ||
            sscanf (argv[2], "%d", &(sharedMemory->c)) != 1 || sharedMemory->c <= 0 ||
            sscanf (argv[3], "%d", &(sharedMemory->agt)) != 1 || sharedMemory->agt < 0 || sharedMemory->agt > 5000 ||
            sscanf (argv[4], "%d", &(sharedMemory->cgt)) != 1 || sharedMemory->cgt < 0 || sharedMemory->cgt > 5000 ||
            sscanf (argv[5], "%d", &(sharedMemory->awt)) != 1 || sharedMemory->awt < 0 || sharedMemory->awt > 5000 ||
            sscanf (argv[6], "%d", &(sharedMemory->cwt)) != 1 || sharedMemory->cwt < 0 || sharedMemory->cwt > 5000) {
                printError (1);
        }
	sharedMemory->adultsRem = sharedMemory->a;
	sharedMemory->childrenRem = sharedMemory->c;
	
	// File reset

	fclose (fopen (FILE_NAME, "wt"));

	// Creating processes
	
	pid_t pidGenerateAdult, pidGenerateChild, pidTmp, *pidChild = NULL, *pidAdult = NULL;
	pidAdult = malloc (sharedMemory->a * sizeof (pid_t));
	
	if ((pidGenerateAdult = fork ()) == 0) {	// Generate adults process
		signal(SIGUSR1, cacthSignalGenProc);
		for (int i = 1; i <= sharedMemory->a; i++) {
			pidTmp = fork ();
			if (pidTmp == 0) {
				newProc (0, i, segmentID);
				break;
			}
			else if (pidTmp < 0);	//error
			else {
				if (sharedMemory->agt != 0) usleep (rand () % (sharedMemory->agt + 1));
			}
		}
		while (wait (NULL) != -1);
		shmdt (sharedMemory);
		exit (0);
	}
	
	// Back in parrent
	else if (pidGenerateAdult < 0) printError (2);
	
	if ((pidGenerateChild = fork ()) == 0) {        // Generate childs process
		signal(SIGUSR1, cacthSignalGenProc);
		for (int i = 1; i <= sharedMemory->c; i++) {
                        pidTmp = fork ();
			if (pidTmp == 0) {
				newProc (1, i, segmentID);
				break;
			}
                        else if (pidTmp < 0);   //error
                        else {
                                if (sharedMemory->cgt != 0) usleep (rand () % (sharedMemory->cgt + 1));
                        }
                }
		while (wait (NULL) != -1);
		shmdt (sharedMemory);
		exit (0);
	}

        // Back in parent
        else if (pidGenerateAdult < 0) {        // fork se nezdari
                wait (NULL);	
		printError (2);
        }
	
	while (wait (NULL) != -1);

	printf ("child zbyva %d\n", sharedMemory->childrenRem);

	sem_unlink (M_SEMAPHORE);	// Cleaning own semaphore
	sem_unlink (A_SEMAPHORE);
	sem_unlink (C_SEMAPHORE);
	sem_unlink (E_SEMAPHORE);
	shmdt (sharedMemory);
	shmctl (segmentID, IPC_RMID, NULL);

	exit (0);
}
