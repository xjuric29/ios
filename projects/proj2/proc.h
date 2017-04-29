#define FILE_NAME "proj2.out"
#define W_SEMAPHORE "xjuric29_write"

struct processControl {
	int a, aAct, agt, awt, c, cAct, cgt, cwt;
};

void newProc (const int type, const int order, const int waitTime, int segmentID);
