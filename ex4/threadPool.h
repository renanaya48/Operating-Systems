/**
* Renana Yanovsky Eichenwald
* 308003862
*/
#include <sys/types.h>
#include "osqueue.h"
#ifndef __THREAD_POOL__
#define __THREAD_POOL__

/**
 * struct task Task
 */
typedef struct task
{
    void(*function)(void*);
    void *args;
}Task;

/**
 * enum errors
 */
typedef enum{
    threadpoolInvalid = -1,
    threadpoolLockFailure = -2,
    threadpoolShutdown = -3,
    threadpoolFail = -4
}threadpool_Error;

/**
 * struct thread_pool ThreadPool
 *
 */
typedef struct thread_pool
{
    OSQueue* tasksQueue;
    pthread_mutex_t lock;
    pthread_cond_t notify;
    pthread_t *thread;
    int maxNumOfThread;
    Task* task;
    int toStop;
    int inProsses;
    int pendingTasks;

}ThreadPool;

/**
 * enum of destroy
 */
typedef enum{
    waitForTasksbeforeShut = 1
}destroy_flag;

ThreadPool* tpCreate(int numOfThreads);

void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks);

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param);

#endif
