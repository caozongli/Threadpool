#include "ThreadPool.h"
#include "ThreadPool.cpp"
#include <unistd.h>
#include <iostream>
using namespace std;

void taskFunc(void* arg)
{
	cout << "thread" << pthread_self() << "is working, number = " << *(int*)arg << endl; 
	sleep(1);
}


int main()
{
	ThreadPool<int> pool(4, 10);
	for(int i = 0; i < 100; ++i)
	{
		int *num = new int;
		*num = i+ 100;
		pool.AddTask(Task<int>(taskFunc, num));
	}
	sleep(20);
	return 0;
}
