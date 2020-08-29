#include <ff/farm.hpp>
#include <ff/allocator.hpp>
#include <iostream>
#include <math.h>
#include <stdlib.h>


using namespace std;
using namespace ff;

int mask = 0;

typedef struct {
    int columns;
    int rightDiagonal;
    int leftDiagonal;
} ff_task_t;


class Emitter: public ff_node {

public:
	int boardsize;
	int depth;
	int numberOfTasks;
	double emitterTsynch;
	double Te;

public:	

	Emitter(int n, int d){ boardsize = n; depth = d; numberOfTasks = 0;  emitterTsynch = 0; Te = 0;}

	int svc_init() {
		mask = (1 << boardsize) - 1;
		return 0;
	}

	inline void generateTree(int leftDiagonal, int columns, int rightDiagonal, int actualRow){

		if(actualRow == depth-1){
			ff_task_t * task = (ff_task_t*)malloc(sizeof(ff_task_t));
			task->columns       = columns;
			task->leftDiagonal  = leftDiagonal;
			task->rightDiagonal = rightDiagonal;

			auto start = std::chrono::high_resolution_clock::now();
			ff_send_out(task);
			auto end = std::chrono::high_resolution_clock::now();

			emitterTsynch += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

			numberOfTasks++;

			return;
		}

		register int rowFreePositions = ~(leftDiagonal | columns | rightDiagonal) & mask;

		while(rowFreePositions){
		   register int lsb = rowFreePositions & -rowFreePositions;
		   rowFreePositions -= lsb;
		   int n = actualRow + 1;
		   generateTree( (leftDiagonal|lsb)<<1, (columns|lsb), (rightDiagonal|lsb)>>1, n);
		}

	}

	 void * svc(void * t) {
	 	auto start = std::chrono::high_resolution_clock::now();
	 	generateTree(0, 0, 0, 0);
	 	auto end = std::chrono::high_resolution_clock::now();

	 	Te = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000;
	 	Te = Te/numberOfTasks;

	 	emitterTsynch = emitterTsynch/1000;

	 	return NULL;
	 }
};	

class Worker: public ff_node {

public:
	unsigned long int workerSolutions;
	double Tw;
	unsigned long int numberOfTasks;

public:

	void _try(int leftDiagonal, int columns, int rightDiagonal){

		register int _leftDiagonal    = leftDiagonal;
		register int _columns 		  = columns;
		register int _rightDiagonal   = rightDiagonal;

		if(columns == mask){
			workerSolutions++;
		}

		register int rowFreePositions = ~(_leftDiagonal | _columns | _rightDiagonal) & mask;

		while(rowFreePositions){  
		   register int lsb = rowFreePositions & -rowFreePositions;
		   rowFreePositions -= lsb;
		   _try( (_leftDiagonal|lsb)<<1, (_columns|lsb), (_rightDiagonal|lsb)>>1 );
		}

	}
	
	 Worker(){ workerSolutions = 0; }

	 int getSolutions(){ return workerSolutions; }

	 void * svc(void * t) {

	 	auto start = std::chrono::high_resolution_clock::now();
	 	ff_task_t * task  = (ff_task_t *)t;
	 	if(task){
	 	  numberOfTasks++;
	 	}
	 	
	 	int leftDiagonal  = task->leftDiagonal,
	 		columns		  = task->columns,
	 		rightDiagonal = task->rightDiagonal;

	 	_try(leftDiagonal, columns, rightDiagonal);

	 	free(task);

	 	auto end = std::chrono::high_resolution_clock::now();
	 	Tw += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count()/1000;

	 	//unsigned long int * solutions = (unsigned long int *)malloc(sizeof(unsigned long int));
	 	//*solutions = workerSolutions;	 	
	 	//ff_send_out(solutions);
	 	//workerSolutions = 0;

        return GO_ON;
    }

};


/*class Collector: public ff_node {

public:
	unsigned long int solutions;
	int nworkers;

public:

	 Collector(int nw){ solutions = 0; nworkers = nw; }

	 void * svc(void * t) {

	        unsigned long int * workerSolutions  = (unsigned long int *)t;

			solutions += *workerSolutions;

	        free(workerSolutions);
	        workerSolutions = NULL;

    		return GO_ON;
    	
       }

    void svc_end(){
    	//cout << endl;
    	//cout << "n. solutions " << solutions << endl;
    	//cout << endl;
    }   

    int getSolutions() const { return solutions; }

};*/

int main(int argc, char* argv[]) {

	int n = atoi(argv[1]),
		nw = atoi(argv[2]),
		d = atoi(argv[3]);

	unsigned long int numberOfSolutions = 0;	

	ff_farm<> farm;

	Emitter E(n, d);
	farm.add_emitter(&E);

	vector<ff_node *> w;
    for(int i=0; i<nw; i++) 
    	w.push_back(new Worker());
    	
    farm.add_workers(w);	
	
	//Collector C(nw);
	//farm.add_collector(&C);

	if (farm.run_and_wait_end()<0) {
        error("running farm\n");
        return -1;
    }

    //reduce solutions
    auto start = std::chrono::high_resolution_clock::now();

	for(int i=0; i<nw; i++) 
    	numberOfSolutions += ((Worker *)w[i])->getSolutions();

    auto end = std::chrono::high_resolution_clock::now();
	double emitter_ts = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

    //farm.ffStats(std::cout);

    cout << endl;
    cout << "n. sol = " << numberOfSolutions << endl;
    cout << "farm completion time = " << farm.ffTime() << " (ms)" << endl;
    cout << endl;

}