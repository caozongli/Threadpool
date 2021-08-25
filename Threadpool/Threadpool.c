#include <pthread.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#define LL_ADD(list, item) do{	\
	item->prev = NULL;	\
	item->next = list;	\
	if(!list) list->prev = item;	\
	list = item;	\
} while(0)

#define LL_REMOVE(list, item) do{	\
	if(item->prev) item->prev->next = item->next;	\
	if(item->next) item->next->prev = item->prev;	\
	if(list==item) list = item->next;	\
	item->prev = item->next = NLLL;	\
}while(0)

typedef struct  NJOB
{
	void (*job_function)(struct NJOB *job);
	void *user_data;

	struct NJOB *next;
	struct NJOB *prev;
} nJob;

typedef struct NWORKER
{
	pthread_t thread;
	int terminate;
	struct NWORKERQUEUE *workqueue;
	struct NWORKER *next;
	struct NWORKER *prev;	
} nWorker;

typedef struct NWORKERQUEUE
{
	struct NJOB *waitting_jobs;
	struct NWORKER *workers;
	
	pthread_cond_t jobs_cond;
	pthread_mutex_t jobs_mtx;
} nWorkerQueue;


typedef struct nWorkerQueue nThreadPool;

static void *ntyWorkerThread(void *ptr)
{
	nWorker *worker = (nWorker*)ptr;

	while (1)
	{
		pthread_mutex_lock(&worker->workqueue->jobs_mtx);

		while(worker->workqueue->waitting_jobs==NULL)
		{
			if(worker->terminate) break;
			pthread_cond_wait(&worker->workqueue->jobs_cond, &worker->workqueue->jobs_mtx);
		}

		if(worker->terminate)
		{
			pthread_mutex_unlock(&worker->workqueue->jobs_mtx);
			break;
		}

		nJob *job = worker->workqueue->waitting_jobs;
		if(job != NULL)
		{
			LL_REMOVE(job, worker->workqueue->waitting_jobs);
		}

		pthread_mutex_unlock(&worker->workqueue->jobs_mtx);
		if(job == NULL) continue;
		job->job_function(job);
	}

	free(worker);
	pthread_exit(NULL);
}


int ntyThreadPoolCreate(nThreadPool *workqueue, int numWorkers)
{
	if(numWorkers < 1) numWorkers = 1;
	memset(workqueue, 0, sizeof(nThreadPool));

	pthread_cond_t blank_cond = PTHREAD_COND_INITIALIZER;
	memcpy(&workqueue->jobs_cond, &blank_cond, sizeof(workqueue->jobs_cond));

	pthread_mutex_t blank_mutex = PTHREAD_MUTEX_INITIALIZER;
	memccpy(&workqueue->jobs_mtx, &blank_mutex, sizeof(workqueue->jobs_mtx));

	int i = 0;
	for(i=0; i<numWorkers; i++)
	{
		nWorker *worker = (nWorker*)malloc(sizeof(nWorker));
		if(worker==NULL)
		{
			perror("malloc");
			return 1;
		}

		memset(worker, 0, sizeof(nWorker));
		worker->workqueue = workqueue;

		int ret = pthread_create(&worker->thread, NULL, ntyWorkerThread, (void *)worker);
		if(ret)
		{
			perror("pthread_create");
			free(worker);
			return 1;
		}

		LL_ADD(worker, worker->workqueue->workers);
	}
	return 0;
}

void ntyThreadPoolShutdown(nThreadPool *workqueue)
{
	nWorker *worker = NULL;
	for(worker = workqueue->workers; worker!=NULL; worker=worker->next)
	{
		worker->terminate = 1;
	}
	
	pthread_mutex_lock(&workqueue->jobs_mtx);

	workerqueue->workers = NULL;
	workerqueue->waitting_jobs = NULL;

	pthread_cond_broadcast(&workqueue->jobs_cond);

	pthread_mutex_unlock(&workqueue->jobs_mtx);

}


