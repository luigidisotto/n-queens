#include <iostream>
#include <chrono>

using namespace std;

int mask = 0;
int count = 0;

inline void _try(int ld, int cols, int rd)
{

	if (cols == mask)
	{
		count++;
	}

	register int poss = ~(ld | cols | rd) & mask;

	while (poss)
	{
		register int lsb = poss & -poss;
		poss -= lsb;
		_try((ld | lsb) << 1, (cols | lsb), (rd | lsb) >> 1);
	}
}

int main(int argc, char *argv[])
{

	int n = atoi(argv[1]);
	mask = (1 << n) - 1;

	auto start = std::chrono::high_resolution_clock::now();
	_try(0, 0, 0);
	auto end = std::chrono::high_resolution_clock::now();

	double t = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

	std::cout << "completion time: " << t << " (ms)" << std::endl;
	//std::cout << t << std::endl;
	//std::cout << "n. solutions: " << count << std::endl;

	return 0;
}