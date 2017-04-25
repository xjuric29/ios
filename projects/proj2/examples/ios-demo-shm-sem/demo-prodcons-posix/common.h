
#ifndef _COMMOH_H
#define _COMMON_H 1

#define _POSIX_C_SOURCE 200809L

#include <stdio.h>

extern FILE *logFile;

#define FNAME "proj2.out"

#define shmKEY   "/ios-proj2"
#define semMUTEX "/ios-proj2-mutex"
#define semITEMS "/ios-proj2-items"
#define semSPACE "/ios-proj2-space"

#define BUFSIZE 1
#define shmSIZE sizeof(int)*BUFSIZE

#endif
