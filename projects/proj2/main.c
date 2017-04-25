#include <stdio.h>
#include <stdlib.h>

void printError (int type) {
	switch (type) {
		case 1:
			fprintf (stderr, "Usage:\n"
				"./proj2 A C AGT CGT AWT CWT\n");
			exit (1);
		case 2:
			exit (2);
	}
}

int main (int argc, char **argv) {
	
	//Arguments parsing
	int A, C, AGT, CGT, AWT, CWT;
	const int numberOfArgs = 7;
	
	if (argc != numberOfArgs || 
	    sscanf (argv[1], "%d", &A) != 1 || A <= 0 || 
	    sscanf (argv[2], "%d", &C) != 1 || C <= 0 ||
	    sscanf (argv[3], "%d", &AGT) != 1 || AGT < 0 || AGT > 5000 ||
            sscanf (argv[4], "%d", &CGT) != 1 || CGT < 0 || CGT > 5000 ||
	    sscanf (argv[5], "%d", &AWT) != 1 || AWT < 0 || AWT > 5000 ||
            sscanf (argv[6], "%d", &CWT) != 1 || CWT < 0 || CWT > 5000) {
		printError (1);
	}
	return 0;
}
