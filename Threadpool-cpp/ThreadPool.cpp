#include "ThreadPool.h"
#include <iostream>
#include <string.h>
#include <string>
#include <stdlib.h>
#include <unistd.h>
using namespace std;

template <typename T>
ThreadPool<T>::ThreadPool(int min, int max)
{
	//实例化任务队列
	taskQ = new TaskQueue<T>;
	do
	{
		if(taskQ == nullptr)
		{
			cout << "new task fail..." << endl;
			break;
		}
		threadIDs = new pthread_t[max];
		if(threadIDs == nullptr)
		{
			cout << "new ThreadIDs fail..." << endl;
			break;
		}
		memset(threadIDs, 0, sizeof(pthread_t)*max);
		minNum = min;
		maxNum = max;
		busyNum = 0;
		liveNum = min;
		exitNum = 0;

		if(pthread_mutex_init(&mutexpool, NULL) !=0 ||
		   pthread_cond_init(&notEmpty, NULL) !=0 )
		{
			cout << "mutex or condition init fail...\n";
			break;
		}

		shutdown = 0;

		//创建线程
		pthread_create(&managerID, NULL, manager, this);
		for(int i=0; i<min; i++)
		{
			pthread_create(&threadIDs[i], NULL, worker, this);
		}
		return;

	}while(0);

	//释放资源
	if(threadIDs) delete []threadIDs;
	if(taskQ) delete taskQ;
}

template <typename T>
ThreadPool<T>::~ThreadPool()
{
	//关闭线程池
	shutdown = 1;
	pthread_join(managerID, NULL);

	for(int i = 0; i < liveNum; i++)
	{
		pthread_cond_signal(&notEmpty);
	}

	if(taskQ) delete taskQ;
	if(threadIDs) delete []threadIDs;
	
	pthread_mutex_destroy(&mutexpool);
	pthread_cond_destroy(&notEmpty);
}

template <typename T>
void ThreadPool<T>::AddTask(Task<T> task)
{
	if(shutdown)
	{
		return;
	}
	taskQ->addTask(task);
	pthread_cond_signal(&notEmpty);
}

template <typename T>
int ThreadPool<T>::getBusyNum()
{
	pthread_mutex_lock(&mutexpool);
	int busyNum = this->busyNum;
	pthread_mutex_unlock(&mutexpool);
	return busyNum;
}

template <typename T>
int ThreadPool<T>::getAliveNum()
{
	pthread_mutex_lock(&mutexpool);
	int liveNum = this->liveNum;
	pthread_mutex_unlock(&mutexpool);
	return liveNum;
}

template <typename T>
void* ThreadPool<T>::worker(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);

	while(1)
	{
		pthread_mutex_lock(&pool->mutexpool);
		//判断当前任务是否为空
		while(!pool->taskQ->taskNumber() && !pool->shutdown)
		{
			//阻塞工作线程
			pthread_cond_wait(&pool->notEmpty, &pool->mutexpool);

			//判断是不是要销毁线程
			if(pool->exitNum > 0)
			{
				pool->exitNum--;
				if(pool->liveNum > pool->minNum)
				{
					pool->liveNum--;
					pthread_mutex_unlock(&pool->mutexpool);
					pool->threadExit();
				}
			}
		}

		if(pool->shutdown)
		{
			pthread_mutex_unlock(&pool->mutexpool);
			pool->threadExit();
		}

		//从任务队列中取出一个任务
		Task<T> task = pool->taskQ->takeTask();

		pool->busyNum++;
		pthread_mutex_unlock(&pool->mutexpool);
		cout << "thread " << to_string(pthread_self()) << " start working..." << endl;

		task.function(task.arg);
		delete task.arg;
		task.arg = nullptr;

		pthread_mutex_lock(&pool->mutexpool);
		pool->busyNum--;
		pthread_mutex_unlock(&pool->mutexpool);

	}

}

template <typename T>
void* ThreadPool<T>::manager(void* arg)
{
	ThreadPool* pool = static_cast<ThreadPool*>(arg);

	while(!pool->shutdown)
	{
		//每隔3s
		sleep(3);
		
		pthread_mutex_lock(&pool->mutexpool);
		int queueSize = pool->taskQ->taskNumber();
		int busyNum = pool->busyNum;
		int liveNum = pool->liveNum;
		pthread_mutex_unlock(&pool->mutexpool);
		
		//添加线程
		//任务个数 > 存活线程数 && 存活线程数 < 最大线程数
		if(queueSize > liveNum && liveNum < pool->maxNum)
		{
			pthread_mutex_lock(&pool->mutexpool);
			int counter = 0;
			
			for(int i = 0; i < pool->maxNum && counter < NUMBER && pool->liveNum < pool->maxNum; ++i)
			{
				if(pool->threadIDs[i] == 0)
				{
					pthread_create(&pool->threadIDs[i], NULL, worker, pool);
					counter++;
					pool->liveNum++;
				}
				
			}
			pthread_mutex_unlock(&pool->mutexpool);
		}

		//销毁线程
		//忙线程*2 < 存活线程数 && 存活线程数 > 最小线程数
		if(busyNum * 2 < liveNum && liveNum > pool->minNum)
		{
			pthread_mutex_lock(&pool->mutexpool);
			pool->exitNum = NUMBER;
			pthread_mutex_unlock(&pool->mutexpool);
			for(int i = 0; i < NUMBER; i++)
			{
				pthread_cond_signal(&pool->notEmpty);
			}
		}

		//唤醒线程
		//任务不为空 && 忙线程 < 存活线程
		if(pool->taskQ->taskNumber() > 0 && busyNum < liveNum)
		{
			pthread_cond_signal(&pool->notEmpty);
		}
	}
	return NULL;
}

template <typename T>
void ThreadPool<T>::threadExit()
{
	pthread_t tid = pthread_self();
	for(int i = 0; i < maxNum; ++i)
	{
		if(threadIDs[i] == tid)
		{
			threadIDs[i] = 0;
			cout << "threadExit() called, " << to_string(tid) << " exiting..." << endl;
			break;
		}
	}
	pthread_exit(nullptr);
}


