/**
* Renana Yanovsky Eichenwald
* 308003862
*/
#include "threadPool.h"
#include <stdio.h>
//#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
//#include <wait.h>

#define ERROR_MSG "Error in system call\n"
#define ERROR_LEN 150

typedef enum {
    immediateShutdown =1,
    waitForTaskSutdown =2
}threadpool_shutdown;

/**
 *  the function write an error msg ans exit
 * @return error massage
 */
void* error() {
    write(2, ERROR_MSG, ERROR_LEN);
    return NULL;
    //EXIT_FAILURE;
}

/**
 * the function make the thread work
 * @param threadPool the threadPool that own the thread
 */
static void* checkThread(void* threadPool){
    ThreadPool* pool = (ThreadPool*)threadPool;
//    Task tasks;
    Task* getTask;
  //  OSNode* current;
    int flag=0;

    for(;;){
        /*
        if(!pool->dest) {
            if (pool->tasksQueue->head != NULL) {
                current = pool->tasksQueue->head;
                pool->dest++;
            }
        }
         */
        pthread_mutex_lock(&(pool->lock));

        //wait on condition variable, check spurious wakeups.
        while((pool->pendingTasks==0)&&(!pool->toStop)){
            pthread_cond_wait(&(pool->notify), (&(pool->lock)));
        }
/*
        if((pool->toStop == immediateShutdown) || ((pool->toStop==waitForTaskSutdown)&&
                (pool->pendingTasks==0))||((pool->toStop==waitForTaskSutdown) &&
                (osIsQueueEmpty(pool->tasksQueue)))){
            break;
        }
        */
        if((pool->toStop == immediateShutdown) ||((pool->toStop==waitForTaskSutdown) &&
                (osIsQueueEmpty(pool->tasksQueue)))){
            break;
        }


        /***********the change******************/
        getTask = (Task*)osDequeue(pool->tasksQueue);
        //unlock
        pthread_mutex_unlock(&(pool->lock));
        if(getTask!=NULL){
            pool->pendingTasks--;
            (*(getTask->function))(getTask->args);
            free(getTask);

        }
        /*
        if(getTask!=NULL){
            tasks.function = (getTask->function);
            tasks.args = getTask->args;
            pool->pendingTasks--;

            //unlock
            pthread_mutex_unlock(&(pool->lock));

            //get to work
            (*(tasks.function))(tasks.args);

        }
         */

        /*****pay attention, work*/
        /*

        if(current!=NULL) {
            //grab task
            getTask = (Task*)(current->data);
            tasks.function = (getTask->function);
            tasks.args = getTask->args;
            current = current->next;
            pool->pendingTasks--;

            //unlock
            pthread_mutex_unlock(&(pool->lock));

            //get to work
            (*(tasks.function))(tasks.args);
        }
         */
        /********************************************/
        else{
            pthread_mutex_unlock(&(pool->lock));

        }
    }
    pool->inProsses--;
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return (NULL);

}

/**
 * the function creates a ThreadPool object
 * @param numOfThreads num of threads to creates
 * @return a newly created threadPool or NULL
 */
ThreadPool* tpCreate(int numOfThreads){
    ThreadPool* pool;
    int i=0;
    Task* taskTemp;

    if(numOfThreads<=0)
        return NULL;

    if((pool=(ThreadPool*)malloc(sizeof(ThreadPool)))==NULL) {
        error();
    }

    //Initialize
    pool->maxNumOfThread=0;
    pool->tasksQueue = osCreateQueue();
    pool->inProsses=0;
    pool->toStop=0;
    pool->pendingTasks = 0;
//    pool->conterSavePlace=0;


    pool->thread = (pthread_t*)malloc(sizeof(pthread_t)*numOfThreads);
    if(pool->thread==NULL)
        error();

    taskTemp= (Task*)malloc(sizeof(Task)*numOfThreads);
    if(taskTemp==NULL)
        error();
    else
        pool->task=taskTemp;
    //int res=pthread_mutex_init(&(pool->lock), NULL);
    //int result=pthread_cond_init(&(pool->notify), NULL);

    if((pthread_mutex_init(&(pool->lock), NULL)!=0)||(pthread_cond_init(&(pool->notify), NULL)!=0)||
        (pool->thread==NULL)||((pool->tasksQueue)==NULL)){
      if(pool){
          tpDestroy(pool, 0);
          error();
      }
      return NULL;
    }

    //start!
    for(; i<numOfThreads; ++i){
        int threadSucceed;
        //threadSucceed=pthread_create(&(pool->thread[i]), NULL, checkThread, (void*)pool);

        if(pthread_create(&(pool->thread[i]), NULL, checkThread, (void*)pool)!=0){

        //if(threadSucceed !=0 ){
            tpDestroy(pool, 0);
            return NULL;
        }
        pool->maxNumOfThread++;
        pool->inProsses++;
    }
    return pool;
}

/**
 * the function inserts a new task to the queue
 * @param threadPool threadPool to add the task
 * @param computeFunc pointer to the function to the task
 * @param param the parameters to the function
 * @return 0 if all went well, neg value if something went wrong
 */
int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    int err =0 ;

    Task* taskSave = (Task*)malloc(sizeof(Task));
    Task* saveTask;
    Task getTask;
    if(computeFunc==NULL||threadPool==NULL){
        return threadpoolInvalid;
    }

    //if function destroy had been called, cant add tasks
    if((threadPool->toStop == immediateShutdown) || (threadPool->toStop==waitForTaskSutdown)){
        //printf("try to add task\n");
        return -1;
    }

    if(pthread_mutex_lock(&(threadPool->lock))!=0){
        return threadpoolLockFailure;
    }


    do
    {
        if(threadPool->toStop){
            err = threadpoolShutdown;
            return -1;
            break;
        }

        /*
        getTask.function=computeFunc;
        getTask.args=param;
        saveTask = &getTask;
        */


        //int temp1=0;

        taskSave->function= (*computeFunc);
        taskSave->args= param;



        osEnqueue(threadPool->tasksQueue, taskSave);


        //osEnqueue(threadPool->tasksQueue, saveTask);


        threadPool->pendingTasks++;
//        threadPool->conterSavePlace++;
        if (pthread_cond_signal(&(threadPool->notify)) != 0) {
            err=threadpoolLockFailure;
            break;
        }
    }while(0);

    if(pthread_mutex_unlock(&(threadPool->lock))!=0){
        err = threadpoolLockFailure;
    }
    return err;
}

/**
 * the function free the memory
 * @param pool the threadPool to free
 * @return 0 if all went well, neg if something went wrong
 */
int freeThreadPool(ThreadPool* pool ){
    int i=0;
    if(pool==NULL||pool->inProsses>0){
        return -1;
    }
    //check if we manage to allocate
    if(pool->thread){
        free(pool->thread);
        free(pool->task);
        osDestroyQueue(pool->tasksQueue);

        pthread_mutex_lock(&(pool->lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->notify));
    }
    free(pool);
    return 0;

}

/**
 * the function destroy the threadPool
 * @param threadPool the threadPool to destroy
 * @param shouldWaitForTasks 0-dont wait for task in the queue, 1 - wait
 */
void tpDestroy(ThreadPool* threadPool, int shouldWaitForTasks) {
    int i = 0, err = 0;

    if (threadPool == NULL) {
        error();
    }
    if (pthread_mutex_lock(&(threadPool->lock)) != 0) {
        error();
    }
    do {
        if (threadPool->toStop) {
            /***********************/
            err = threadpoolShutdown;
            error();
            break;
        }
        threadPool->toStop = (shouldWaitForTasks & waitForTasksbeforeShut) ?
                             waitForTaskSutdown : immediateShutdown;

        //wake up all worker threads
        if ((pthread_cond_broadcast(&(threadPool->notify)) != 0) ||

            (pthread_mutex_unlock(&(threadPool->lock)) != 0)) {
            /*********/
            err = threadpoolLockFailure;
            error();
            break;
        }

        //join all worker threads
        for(i=0; i< threadPool->maxNumOfThread; i++){
            if(pthread_join(threadPool->thread[i], NULL)!=0){
                err=threadpoolFail;
                error();
            }
        }
/*
        //join all worker threads
        OSNode *temp = threadPool->tasksQueue->head;
        while (temp != NULL) {
            if (pthread_join(threadPool->thread[i++], NULL) != 0) {
                err = threadpoolFail;
                error();
            }
            temp = temp->next;
        }
*/
    } while (0);

    //if everything went well - deallocate the threadpool
    if(!err)
        freeThreadPool(threadPool);

}




