//
// Created by renana on 02/05/19.
//
#include "threadPool.h"
#include <stdio.h>
//#include <malloc.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

#define ERROR_MSG "Error in system call\n"
#define ERROR_LEN 150

typedef enum {
    immediateShutdown =1,
    waitForTaskSutdown =2
}threadpool_shutdown;

void* error() {
    write(2, ERROR_MSG, ERROR_LEN);
    return NULL;
    //EXIT_FAILURE;
}

static void* checkThread(void* threadPool){
    ThreadPool* pool = (ThreadPool*)threadPool;
    Task tasks;

    for(;;){
        pthread_mutex_lock(&(pool->lock));

        //wait on condition variable, check spurious wakeups.
        while((pool->maxNumOfThread==0)&&(!pool->toStop)){
            pthread_cond_wait(&(pool->notify), &(pool->lock));
        }

        if((pool->toStop == immediateShutdown) || ((pool->toStop==waitForTaskSutdown)&&
                (pool->maxNumOfThread==0))){
            break;
        }

        //grab task
        tasks.function=pool->task->function;
        tasks.args=pool->task->args;
        pool->maxNumOfThread-=1;

        //unlock
        pthread_mutex_unlock(&(pool->lock));

        //get to work
        (*(tasks.function))(tasks.args);
    }
    pool->inProsses--;
    pthread_mutex_unlock(&(pool->lock));
    pthread_exit(NULL);
    return (NULL);

}

ThreadPool* tpCreate(int numOfThreads){
    ThreadPool* pool;
    int i=0;

    if((pool=(ThreadPool*)malloc(sizeof(ThreadPool)))==NULL){
        error();
    }

    //Initialize
    pool->maxNumOfThread=0;
    pool->tasksQueue = osCreateQueue();
    pool->inProsses=0;
    pool->toStop=0;

    pool->thread = (pthread_t*)malloc(sizeof(pthread_t)*numOfThreads);
    if(pool->thread==NULL)
        error();
    pool->task=(Task*)malloc(sizeof(Task)*numOfThreads);
    if(pool->task==NULL)
        error();
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
        if(pthread_create(&(pool->thread[i]), NULL, checkThread, (void*)pool)!=0 ){
            tpDestroy(pool, 0);
            return NULL;
        }
        pool->maxNumOfThread++;
        pool->inProsses++;
    }
    return pool;
}

int tpInsertTask(ThreadPool* threadPool, void (*computeFunc) (void *), void* param){
    int err =0 ;

    if(computeFunc==NULL||threadPool==NULL){
        return threadpoolInvalid;
    }
/*
    if(pthread_mutex_lock(&(threadPool->lock))!=0){
        return threadpoolLockFailure;
    }

*/
    do
    {
        if(threadPool->toStop){
            err = threadpoolShutdown;
            break;
        }
        threadPool->task->function = computeFunc;
        threadPool->task->args = param;
        osEnqueue(threadPool->tasksQueue, threadPool->task);
        if (pthread_cond_signal(&(threadPool->notify)) != 0) {
            err=threadpoolLockFailure;
            break;
        }
    }while(0);

    if(pthread_mutex_unlock(&threadPool->lock)!=0){
        err = threadpoolLockFailure;
    }
    return err;
}

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
            break;
        }
        threadPool->toStop = (shouldWaitForTasks & waitForTasksbeforeShut) ?
                             waitForTaskSutdown : immediateShutdown;

        //wake up all worker threads
        if ((pthread_cond_broadcast(&(threadPool->notify)) != 0) ||

            (pthread_mutex_unlock(&(threadPool->lock)) != 0)) {
            /*********/
            error();
            break;
        }

        //join all worker threads
        OSNode *temp = threadPool->tasksQueue->head;
        while (temp != NULL) {
            if (pthread_join(threadPool->thread[i++], NULL) != 0) {
                error();
            }
            temp = temp->next;
        }

    } while (0);

    //if everything went well - deallocate the threadpool
    freeThreadPool(threadPool);

}




