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

void printError (int type);
void cacthSignalGenProc (int signal);
void cacthSignalMainProc (int signal);

// Globals

int segmentID;
processControl *sharedMemory;

int main (int argc, char **argv) {

	// Set behavior for signals
        signal(SIGUSR2, cacthSignalMainProc);
        signal(SIGTERM, cacthSignalMainProc);
        signal(SIGINT, cacthSignalMainProc);
	
	// Shared memory

	segmentID = shmget (ftok (PATH, 1), sizeof (processControl), IPC_CREAT | S_IRUSR | S_IWUSR);	// See proc.h for more details about shared structure
	
	if (segmentID < 0) cacthSignalMainProc (SIGTERM);
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
	
	pid_t pidGenerateAdult, pidGenerateChild, pidTmp;
	
	if ((pidGenerateAdult = fork ()) == 0) {	// Generate adults process
		
		// Set behavior for signals

                signal(SIGUSR1, cacthSignalGenProc);
                signal(SIGUSR2, cacthSignalGenProc);
                signal(SIGTERM, cacthSignalGenProc);
                signal(SIGINT, cacthSignalGenProc);

		for (int i = 1; i <= sharedMemory->a; i++) {
			pidTmp = fork ();
			if (pidTmp == 0) {
				newProc (0, i, segmentID);
				break;
			}
			else if (pidTmp < 0) {
				cacthSignalGenProc (SIGUSR2);   
                                break;
			}
			else {
				if (sharedMemory->agt != 0) usleep (rand () % (sharedMemory->agt + 1));
			}
		}
		while (wait (NULL) != -1);
		shmdt (sharedMemory);
		exit (0);
	}
	
	// Back in parrent
	else if (pidGenerateAdult < 0) cacthSignalMainProc (SIGTERM);
	
	if ((pidGenerateChild = fork ()) == 0) {        // Generate childs process
		
		// Set behavior for signals

		signal(SIGUSR1, cacthSignalGenProc);
		signal(SIGUSR2, cacthSignalGenProc);
		signal(SIGTERM, cacthSignalGenProc);
		signal(SIGINT, cacthSignalGenProc);

		for (int i = 1; i <= sharedMemory->c; i++) {
                        pidTmp = fork ();
			if (pidTmp == 0) {
				newProc (1, i, segmentID);
				break;
			}
                        else if (pidTmp < 0) {
				cacthSignalGenProc (SIGUSR2);
				break;		
			}
                        else {
                                if (sharedMemory->cgt != 0) usleep (rand () % (sharedMemory->cgt + 1));
                        }
                }
		while (wait (NULL) != -1);
		shmdt (sharedMemory);
		exit (0);
	}

        // Back in parent
        else if (pidGenerateAdult < 0) cacthSignalMainProc (SIGTERM);
	
	//printf ("main pid: %d\n", getpid());	// Debug

	while (wait (NULL) != -1);

	sem_unlink (M_SEMAPHORE);	// Cleaning own semaphore
	sem_unlink (A_SEMAPHORE);
	sem_unlink (C_SEMAPHORE);
	sem_unlink (E_SEMAPHORE);
	shmdt (sharedMemory);
	shmctl (segmentID, IPC_RMID, NULL);

	exit (0);
}

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

void cacthSignalGenProc (int signal) {

        // Signals

        pid_t ppid = getppid ();

        switch (signal) {
                case SIGUSR1:   // Synchronizes finish of child/adult processes
                        if (sharedMemory->adultsRem == 0 && sharedMemory->childrenRem == 0 ) {
                                sem_t *semE = sem_open (E_SEMAPHORE, O_CREAT, S_IWUSR | S_IRUSR, 0);
                                for (int i = 0; i < (sharedMemory->a + sharedMemory->c); i++) {
                                        sem_post (semE);
                                }
                                sem_close (semE);
                        }
                        break;
                case SIGUSR2:
                        kill (ppid, SIGUSR2);
                        break;
                case SIGTERM:
                        kill (0, SIGTERM);
			while (wait (NULL) != -1);
                	shmdt (sharedMemory);
                        exit (2);
                case SIGINT:
                        break;
        }
}

void cacthSignalMainProc (int signal) {
	
	// Signals

	switch (signal) {
		case SIGUSR2:
		case SIGTERM:
		case SIGINT:
			kill (0, SIGTERM);
			while (wait (NULL) != -1);
			sem_unlink (M_SEMAPHORE);       // Cleaning own semaphore
        		sem_unlink (A_SEMAPHORE);
        		sem_unlink (C_SEMAPHORE);
        		sem_unlink (E_SEMAPHORE);
        		shmdt (sharedMemory);
        		shmctl (segmentID, IPC_RMID, NULL);
			printError (2);
	}
}
