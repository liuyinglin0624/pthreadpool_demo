#ifndef MYTHREADPOOL_H
#define MYTHREADPOOL_H

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<string.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<pthread.h>
#include<signal.h>
#include<errno.h>
#include<arpa/inet.h>

class ITask
{
public:
    virtual void TaskJob() = 0;
};

class MyThreadPool
{
public:
    MyThreadPool();
public:
    void pthread_pool_create(int max,int min,int queue_max);
    void pthread_pool_destroy();
    void pthread_pool_addtask(ITask* task);
private:
    static void* pthread_pool_customer(void* arg);
    static void* pthread_pool_manager(void* arg);
    static bool pthread_pool_alive(pthread_t tid);
private:
    int thread_max;
    int thread_min;
    int thread_busy;
    int thread_alive;
    bool thread_shutdown;
    int thread_exitcode;
    int queue_max;
    int queue_size;
    int queue_front;
    int queue_rear;
    pthread_mutex_t lock;
    pthread_cond_t con_productor;
    pthread_cond_t con_customer;
    pthread_t* tids;
    pthread_t managerid;
    ITask** queue_task;
};

#endif // MYTHREADPOOL_H
