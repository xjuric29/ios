#define FILE_NAME "proj2.out"
#define PATH "./proj2" 
#define M_SEMAPHORE "xjuric29_mutex"
#define A_SEMAPHORE "xjuric29_childs"
#define C_SEMAPHORE "xjuric29_adults"
#define E_SEMAPHORE "xjuric29_end"

struct processControl {
	int a, agt, awt, c, cgt, cwt;	// Program params
	int count, children, adults, waiting, leaving, adultsRem, childrenRem;	// Vars for child and adult processes
	
	//count: number of operation
	//children: number of children in the center
	//adults: number of adults in the center
	//waiting: number of children waiting for enter to the center
	//leaving: number of adults waiting for leave from the center 
	//adultsRem: number of adults who are still coming to the center
	//childrenRem: number of children who are still coming to the center
};

void cacthSignalGenProc (int signal);

void newProc (const int type, const int order, int segmentID);
