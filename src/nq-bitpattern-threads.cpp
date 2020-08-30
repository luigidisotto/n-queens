#include <iostream>
#include <vector>
#include <thread>
#include <math.h>
#include "shared_queue.hpp"

using namespace std;

#define EOS ((Task *)NULL)

int mask = 0;

#ifdef STATS
std::mutex coutMutex;
double T_calc = 0;	//worker avg total computing time only
double T_synch = 0; //worker total synch time
double T_calc_emitter = 0;
double T_synch_emitter = 0;
//unsigned long int w_tasks = 0;
//unsigned long int e_tasks = 0;
#endif

class Task
{

public:
	int leftDiagonal;
	int columns;
	int rightDiagonal;

public:
	Task(int l, int c, int r) : leftDiagonal(l), columns(c), rightDiagonal(r) {}
};

class Emitter
{

public:
	vector<SharedQueue<Task *>> &outQueue;
	int receiverWorkerID;
	int numberOfWorkers;
	unsigned long int emitterNumberOfTasks;
	int partialTreesDepth;
	double emitterTcalc;
	double emitterTsynch;

public:
	Emitter(vector<SharedQueue<Task *>> &out, int d, int id, int nw) : outQueue(out)
	{

		emitterNumberOfTasks = 0;
		partialTreesDepth = d;
		receiverWorkerID = 0;
		numberOfWorkers = nw;
		emitterTcalc = 0;
		emitterTsynch = 0;
	}

	inline void buildPromisingTrees(int leftDiagonal, int columns, int rightDiagonal, int actualRow)
	{

		if (actualRow == partialTreesDepth - 1)
		{
			auto start = std::chrono::high_resolution_clock::now();
			outQueue[receiverWorkerID].push(new Task(leftDiagonal, columns, rightDiagonal));
			auto end = std::chrono::high_resolution_clock::now();

			emitterTsynch += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

			receiverWorkerID = (receiverWorkerID + 1) % numberOfWorkers; //round robin scheduling
			emitterNumberOfTasks++;
			return;
		}

		register int rowFreePositions = ~(leftDiagonal | columns | rightDiagonal) & mask;

		while (rowFreePositions)
		{
			register int lsb = rowFreePositions & -rowFreePositions;
			rowFreePositions -= lsb;
			int n = actualRow + 1;
			buildPromisingTrees((leftDiagonal | lsb) << 1, (columns | lsb), (rightDiagonal | lsb) >> 1, n);
		}
	}

	void operator()()
	{
		auto start = std::chrono::high_resolution_clock::now();

		buildPromisingTrees(0, 0, 0, 0);

		auto end = std::chrono::high_resolution_clock::now();

		emitterTcalc = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

		for (int i = 0; i < numberOfWorkers; i++)
		{
			auto startSynch = std::chrono::high_resolution_clock::now();
			outQueue[i].push(EOS);
			auto endSynch = std::chrono::high_resolution_clock::now();
			emitterTsynch += std::chrono::duration_cast<std::chrono::microseconds>(endSynch - startSynch).count();
		}

		emitterTsynch = emitterTsynch / 1000;
		emitterTcalc = emitterTcalc / 1000;
		emitterTcalc = emitterTcalc - emitterTsynch;

#ifdef STATS
		{
			std::lock_guard<std::mutex> lock(coutMutex);
			T_calc_emitter = emitterTcalc;
			T_synch_emitter = emitterTsynch;
			//T_e = emitterTcalc + emitterTsynch;
			//e_tasks = emitterNumberOfTasks;
			//cout << endl;
			//cout << "emitter: " << endl;
			//cout << "   completion time = " << emitterTcalc + emitterTsynch << " (ms) " << endl;
			//cout << "   Tcal = " << fabs((emitterTcalc - emitterTsynch)) << " (ms)" << endl;
			//cout << "   Tsynch =  " << emitterTsynch << " (ms)" << endl;
			//cout << emitterTcalc << " " << emitterTsynch << endl;
			//cout << "   n. tasks = " << emitterNumberOfTasks << endl;
		}
#endif
	}
};

class Worker
{

public:
	unsigned long int *workerSolutions,
		totalWorkerSolutions;
	int workerNumberOfTasks;
	int workerId;
	SharedQueue<Task *> *inQueue;
	double workerTcalc;
	double workerTsynch;

public:
	Worker(SharedQueue<Task *> *_inQueue, int id,
		   unsigned long int *solutions) : inQueue(_inQueue), workerSolutions(solutions)
	{ //, outQueue(_outQueue) {

		totalWorkerSolutions = 0;
		workerNumberOfTasks = 0;
		workerId = id;
		workerTcalc = 0;
		workerTsynch = 0;
	}

	void operator()()
	{

		Task *task = NULL;

		auto start = std::chrono::high_resolution_clock::now();

		while (true)
		{

			auto startSynch = std::chrono::high_resolution_clock::now();
			task = inQueue->pop();
			auto endSynch = std::chrono::high_resolution_clock::now();

			workerTsynch += std::chrono::duration_cast<std::chrono::microseconds>(endSynch - startSynch).count();

			if (task == EOS)
				break;

			workerNumberOfTasks++;

			int leftDiagonal = task->leftDiagonal,
				columns = task->columns,
				rightDiagonal = task->rightDiagonal;

			treeExploring(leftDiagonal, columns, rightDiagonal);

			delete task;
		}

		*workerSolutions = totalWorkerSolutions;

		auto end = std::chrono::high_resolution_clock::now();
		workerTcalc += std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

		workerTcalc = workerTcalc / 1000;
		workerTsynch = workerTsynch / 1000;
		workerTcalc = workerTcalc - workerTsynch;

#ifdef STATS
		{
			std::lock_guard<std::mutex> lock(coutMutex);
			T_calc += workerTcalc;
			T_synch += workerTsynch;
			//cout << endl;
			//cout << "worker " << workerId << endl;
			//cout << "  completion time = " << workerTsynch + workerTcalc << " (ms)" << endl;
			//cout << "  Tcalc = " << workerTcalc << " (ms)" << endl;
			//cout << "  Tsynch = " << workerTsynch << " (ms)" << endl;
			//cout << "  n. tasks = " << workerNumberOfTasks << endl;
			//cout << "  bandwidth = " << (1./(workerTcalc))*workerNumberOfTasks*1.e+6 << " tasks/sec " << endl;
			//cout << "  n. sols = " << totalWorkerSolutions << endl;
		}
#endif
	}

	inline void treeExploring(int leftDiagonal, int columns, int rightDiagonal)
	{

		register int _leftDiagonal = leftDiagonal;
		register int _columns = columns;
		register int _rightDiagonal = rightDiagonal;

		if (columns == mask)
		{
			totalWorkerSolutions++;
		}

		register int rowFreePositions = ~(_leftDiagonal | _columns | _rightDiagonal) & mask;

		while (rowFreePositions)
		{
			register int lsb = rowFreePositions & -rowFreePositions;
			rowFreePositions -= lsb;
			treeExploring((_leftDiagonal | lsb) << 1, (_columns | lsb), (_rightDiagonal | lsb) >> 1);
		}
	}
};

int main(int argc, char *argv[])
{

	int board_size = 0,
		nw = 0,
		depth = 0;
		mask = 0;

	if (argc != 4)
	{
		cout << "usage: <board size> <number of workers> <tree depth>" << endl;
		return -1;
	}
	else
	{
		board_size = atoi(argv[1]);
		nw = atoi(argv[2]);
		depth = atoi(argv[3]);
		mask = (1 << board_size) - 1;
	}

	if (depth > board_size || depth < 2)
		depth = 2;

	vector<SharedQueue<Task *>> emitterToWorkerQueue;

	for (int i = 1; i <= nw; i++)
		emitterToWorkerQueue.push_back(SharedQueue<Task *>());

	thread emitter(Emitter(emitterToWorkerQueue, depth, 0, nw));

	std::vector<thread> workers;

	std::vector<unsigned long int> solutions(nw, 0);

	for (int i = 1; i <= nw; i++)
		workers.push_back(std::thread(Worker(&emitterToWorkerQueue[i - 1], i, &solutions[i - 1])));

	auto start = std::chrono::high_resolution_clock::now();

	emitter.join();

	for (auto &w : workers)
		w.join();

	unsigned long int numberOfSolutions = 0;

	auto start_e = std::chrono::high_resolution_clock::now();
	for (int i = 0; i < nw; i++)
	{
		numberOfSolutions += solutions[i];
	}
	auto end_e = std::chrono::high_resolution_clock::now();
	double emitter_ts = std::chrono::duration_cast<std::chrono::microseconds>(end_e - start_e).count();
	//cout << "[thread] collector service time " << emitter_ts << " (us)" << endl;
	//cout << endl;

	auto end = std::chrono::high_resolution_clock::now();
	double farm_completionTime = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();

	cout << endl;
	cout << "n. sol = " << numberOfSolutions << endl;
	cout << "farm completion time = " << farm_completionTime / 1000 << " (ms)" << endl;
	cout << endl;

#ifdef STATS
	//double tw = (T_w/nw)/(w_tasks/nw);
	//cout << "[thread] avg n. tasks per thread = " << (w_tasks/nw) << endl;
	//cout << "[thread] worker avg service time = " << tw << endl;
	//cout << "[thread] emitter avg service time = " << (T_e/e_tasks) << endl;
	//cout << (T_e/e_tasks)*nw << " " << tw << endl;
	//cout << (tw / ((T_e/e_tasks)*nw)) << endl;
	cout << T_calc / nw << " " << T_synch / nw << " " << T_calc_emitter << " " << T_synch_emitter << endl;
#endif

	return 0;
}