/*
#include <stdio.h>
#include <stdlib.h>
//#include <zconf.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include "osqueue.h"
#include "threadPool.h"

#define THREADS_COUNT 4
#define TASKS_PER_THREAD 30
#define TASKS_PRI_THREAD 10
#define TP_WORKER_THREADS 3

#define DEBUG 1 // Change to 1 for debug info

pthread_mutex_t TasksDoneLock;
pthread_mutex_t TasksInsertedLock;

volatile int tasksDoneCount;
volatile int tasksInsertedCount;

void incTaskAdded() {
    pthread_mutex_lock(&TasksInsertedLock);
    tasksInsertedCount++;
    pthread_mutex_unlock(&TasksInsertedLock);
}

void incTaskDone() {
    pthread_mutex_lock(&TasksDoneLock);
    tasksDoneCount++;
    pthread_mutex_unlock(&TasksDoneLock);
}

int getCurrentThread() {
    return ((long) pthread_self() % 1000);
}


void task1(void *_) {
    int r;
    r = (rand() % 100) + 20;
    if (DEBUG) printf("TASK1 thread %d. sleeping %dms\n",(getCurrentThread()),r);
    usleep(r);
    incTaskDone();
}

void task2(void *_) {
    if (DEBUG) printf("TASK2 thread %d\n",getCurrentThread());
    incTaskDone();
}

void* poolDestoyer (void *arg) {
    ThreadPool* pool = (ThreadPool*) arg;
    tpDestroy(pool,1);
    return NULL;
}

void* tasksAdder(void *arg)
{
    ThreadPool* pool = (ThreadPool*) arg;
    int i;

    for(i=0; i<TASKS_PER_THREAD; ++i) {
        if (!tpInsertTask(pool, task1, NULL)) {
            incTaskAdded();
        }
    }

    return NULL;
}

int shouldWaitForTasksTest(int shouldWait) {
    pthread_t t1[THREADS_COUNT];
    int i,j,result;
    ThreadPool* tp = tpCreate(TP_WORKER_THREADS);
    for (j=0; j<THREADS_COUNT; j++) {
        pthread_create(&t1[j], NULL, tasksAdder, tp);
    }

    for (i=0; i<TASKS_PRI_THREAD ; i++) {
        if (!tpInsertTask(tp, task2, NULL)) {
            incTaskAdded();
        }
    }

    if (DEBUG) printf("-->tp will be destroyed!<--\n");
    tpDestroy(tp,shouldWait);
    if (DEBUG) printf("-->tp destroyed!<--\n");

    if (DEBUG) printf("waiting for other threads to end..\n");
    for (j=0; j<THREADS_COUNT; j++) {
        pthread_join(t1[j], NULL);
    }
    pthread_mutex_lock(&TasksInsertedLock);
    pthread_mutex_lock(&TasksDoneLock);
    if (DEBUG) printf("\nSUMMRAY:\nTasks inserted:%d\nTasks done:%d\n",tasksInsertedCount,tasksDoneCount);
    if (DEBUG) printf("Graceful? %d\n",shouldWait);
    if ((shouldWait && tasksInsertedCount == tasksDoneCount) ||
        (!shouldWait && tasksInsertedCount != tasksDoneCount)) {
        result = 0;
    } else {
        result = 1;
    }

    tasksDoneCount = 0;
    tasksInsertedCount = 0;
    pthread_mutex_unlock(&TasksInsertedLock);
    pthread_mutex_unlock(&TasksDoneLock);

    return result;
}

int insertAfterDestroyTest() {
    ThreadPool* tp = tpCreate(TP_WORKER_THREADS);
    int i;
    //usleep(50);
    for (i=0; i<TASKS_PRI_THREAD ; i++) {
        tpInsertTask(tp, task1, NULL);
    }
    tpDestroy(tp,1);
    return !tpInsertTask(tp, task1, NULL);
}

int doubleDestroy() {
    pthread_t t1;
    ThreadPool *tp = tpCreate(TP_WORKER_THREADS);
    int i;
    for (i = 0; i < TASKS_PRI_THREAD; i++) {
        tpInsertTask(tp, task1, NULL);
    }
    printf("Going to destroy pool from 2 different threads...\n");
    pthread_create(&t1, NULL, poolDestoyer, tp);
    tpDestroy(tp,1);
    pthread_join(t1,NULL);
    printf("Done, did anything break?\n");
    return 0;
}

int main() {
    srand(time(NULL));
    pthread_mutex_init(&TasksDoneLock, NULL);
    pthread_mutex_init(&TasksInsertedLock, NULL);
    tasksDoneCount = 0;
    tasksInsertedCount = 0;
    int i;
    printf("---Tester Running---\n");

    for (i=0; i<10; i++) {
        if (insertAfterDestroyTest())
            printf("Could insert task after tp destroyed!\n");
        if (shouldWaitForTasksTest(0))
            printf("Failed on shouldWaitForTasks = 0, tasks created = tasks done. This should rarely happen..\n");
        if (shouldWaitForTasksTest(1))
            printf("Failed on destroy with shouldWaitForTasks = 1. tasks created != tasks done\n");
    }

    doubleDestroy();

    printf("---Tester Done---\n");
    return 0;
}


*/



#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include "osqueue.h"
#include "threadPool.h"

int i=0;

void hello (void* a)
{

    printf("hello , %d\n", i);
    i++;
}


void test_thread_pool_sanity()
{
    int i;

    ThreadPool* tp = tpCreate(1);

    // tpInsertTask(tp,hello,NULL);



    for(i=0; i<5; ++i)
    {
        tpInsertTask(tp,hello,NULL);
    }

    printf("destroy\n");
    tpDestroy(tp,1);
    printf("destroy1\n");
//    tpInsertTask(tp, hello, NULL);
  //  printf("destroy2\n");


}


int main()
{
    test_thread_pool_sanity();

    return 0;
}



/*

#include <stdio.h>
#include <malloc.h>
#include <unistd.h>
#include "threadPool.h"

void function1();
void function2();
void function3();
void function1WithSleep();
void function2Sleep();

*/
/*
//המספרים צריכים להיות מודפסים בצורה סנכרונית, קודם 11..1 ואז 2...2 ואז 3...3
int main() {
    ThreadPool* threadPool =tpCreate(1);
    char * args = (char *)malloc(10);
    tpInsertTask(threadPool,function1,args);
    tpInsertTask(threadPool,function2,args);
    tpInsertTask(threadPool,function3,args);
    int temp;
    printf("done\n");
    scanf("%d",&temp);
    return 0;
}

*/
/*

//מספרים מודפסים באיזה סדר שבא להם
int main() {
    ThreadPool* threadPool =tpCreate(3);
    char * args = (char *)malloc(10);
    tpInsertTask(threadPool,function1,args);
    tpInsertTask(threadPool,function2,args);
    tpInsertTask(threadPool,function3,args);
    int temp;
    scanf("%d",&temp);
    return 0;
}
*/

/*
//מספרים מודפסים באיזה סדר שבא להם
int main() {
    ThreadPool* threadPool =tpCreate(3);
    char * args = (char *)malloc(10);
    tpInsertTask(threadPool,function1WithSleep,args);
    tpInsertTask(threadPool,function2Sleep,args);
    tpInsertTask(threadPool,function3,args);
    int temp;
    scanf("%d",&temp);
    return 0;
}
*/

/*
//דסטרוי רגיל, חשוב לשים לב שכל הפונקציות מסתיימות לפני הדסטרוי
int main() {
    ThreadPool *threadPool = tpCreate(1);
    char *args = (char *) malloc(10);


    tpInsertTask(threadPool, function1, args);
    tpInsertTask(threadPool, function2, args);
    tpInsertTask(threadPool, function3, args);
    printf("destroy\n");

    tpDestroy(threadPool, 0);
    int temp;
    printf("done\n");
    scanf("%d", &temp);
    return 0;
}
*/

/*

//דסטרוי רגיל, רק המתודה הראשונה אמורה להתבצע
int main() {
    ThreadPool *threadPool = tpCreate(1);
    char *args = (char *) malloc(10);

    tpInsertTask(threadPool, function1WithSleep, args);
    tpInsertTask(threadPool, function2Sleep, args);
    tpInsertTask(threadPool, function3, args);

    tpDestroy(threadPool, 0);
    int temp;
    printf("done\n");
    scanf("%d", &temp);
    return 0;
}

*/
/*
//דסטרוי מיוחד, יש לוודא שכל המשימות מתבצעות!!
int main() {
    ThreadPool *threadPool = tpCreate(1);
    char *args = (char *) malloc(10);

    tpInsertTask(threadPool, function1WithSleep, args);
    tpInsertTask(threadPool, function2Sleep, args);
    tpInsertTask(threadPool, function3, args);
    tpDestroy(threadPool, 1);
    int temp;
    printf("done\n");
    scanf("%d", &temp);
    return 0;
}

*/

/*
//בדיקה שאחרי שקראנו לדסטרוי לא ניתן להוסיף משימות
int main() {
    ThreadPool *threadPool = tpCreate(1);
    char *args = (char *) malloc(10);

    tpInsertTask(threadPool, function1WithSleep, args);
    tpInsertTask(threadPool, function2Sleep, args);
    printf("%d",tpInsertTask(threadPool, function3, args));

    tpDestroy(threadPool, 1);
    printf("%d",tpInsertTask(threadPool, function3, args));


    int temp;
    scanf("%d", &temp);
    return 0;
}

*/
/*void hello (void* a)
{
    printf("hello\n");
    sleep(3);
}


void test_thread_pool_sanity()
{
    int i;

    ThreadPool* tp = tpCreate(5);

    for(i=0; i<10; ++i)
    {
        tpInsertTask(tp,hello,NULL);
    }
    sleep(5);
    tpDestroy(tp,120);
}
int main(){
    test_thread_pool_sanity();

}*/



/*void hello (void* a)
{
    printf("hello\n");
}


void test_thread_pool_sanity()
{
    int i;

    ThreadPool* tp = tpCreate(5);

    for(i=0; i<5; ++i)
    {
        tpInsertTask(tp,hello,NULL);
    }

    tpDestroy(tp,1);
}


int main()
{
    test_thread_pool_sanity();

    return 0;
}*/

void function3() {
    int i;
    for(i=1; i<5;i++) {
        printf("3\n");

    }
}

void function1() {
    int i;
    for(i=1; i<5;i++) {
        printf("1\n");

    }
}
void function1WithSleep() {
    int i;
    for(i=1; i<5;i++) {
        printf("1\n");
        sleep(1);

    }
}

void function2() {
    int i;
    for(i=1; i<5;i++) {
        printf("2\n");
    }
}
void function2Sleep() {
    int i;
    for(i=1; i<5;i++) {
        printf("2\n");
        sleep(1);
    }
}


void* function3Sleep() {
    int i;
    for(i=1; i<5;i++) {
        printf("3\n");
        sleep(1);

    }
}