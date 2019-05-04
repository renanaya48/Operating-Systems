#include <sys/types.h>
#include "osqueue.h"
#ifndef __THREAD_POOL__
#define __THREAD_POOL__

typedef struct task
{
    void(*function)(void*);
    void *args;
}Task;

typedef enum{
    threadpoolInvalid = -1,
    threadpoolLockFailure = -2,
    threadpoolFULLQueue = -3,
    threadpoolShutdown = -4,
    threadpoolFail = -5
}threadpool_Error;

typedef struct thread_pool
{
    OSQueue* tasksQueue;
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *thread;
    int maxNumOfThread;
    int toStop;
    int inProsses;
    Task* task;

}ThreadPool;

typedef enum{
    waitForTasksbeforeShut = 1
}destroy_flag;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
