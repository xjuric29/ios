#define FILE_NAME "proj2.out"
#define M_SEMAPHORE "xjuric29_mutex"
#define A_SEMAPHORE "xjuric29_childs"
#define C_SEMAPHORE "xjuric29_adults"
#define COUNT 0
#define CHILDREN 1
#define ADULTS 2
#define WAITING 3
#define LEAVING 4
#define ADULTS_REM 5

struct processControl {
	int a, aAct, agt, awt, c, cAct, cgt, cwt;
};

void newProc (const int type, const int order, const int waitTime, int segmentID);
