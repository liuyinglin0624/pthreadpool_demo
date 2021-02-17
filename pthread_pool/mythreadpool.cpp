#include "mythreadpool.h"

MyThreadPool::MyThreadPool()
{

}

void MyThreadPool::pthread_pool_create(int max,int min,int queue_max)
{
    this->thread_shutdown = true;
    this->thread_max = max;
    this->thread_min = min;
    this->thread_busy = 0;
    this->thread_alive = 0;
    this->thread_exitcode = 0;
    this->queue_max = queue_max;
    this->queue_size = 0;
    this->queue_front = 0;
    this->queue_rear = 0;
    if(pthread_mutex_init(&this->lock,NULL)||pthread_cond_init(&this->con_customer,NULL) || pthread_cond_init(&this->con_productor,NULL))
    {
        perror("pthread_pool_create init error\n");
        return ;
    }
    if((this->tids = (pthread_t*)malloc(sizeof(pthread_t) * max)) == NULL)
    {
        perror("pthread_pool_create malloc tids error\n");
        return ;
    }
    bzero(this->tids,sizeof(pthread_t) * max);

    if((this->queue_task = (ITask**)malloc(sizeof(ITask*)*queue_max)) == NULL)
    {
        perror("pthread_pool_create malloc queue_task error\n");
        return ;
    }

    for(int i = 0;i<min;i++)
    {
        pthread_create(&this->tids[i],NULL,pthread_pool_customer,this);
        this->thread_alive++;
    }
    pthread_create(&this->managerid,NULL,pthread_pool_manager,this);
}

void MyThreadPool::pthread_pool_destroy()
{
    pthread_mutex_destroy(&this->lock);
    pthread_cond_destroy(&this->con_customer);
    pthread_cond_destroy(&this->con_productor);
    free(this->tids);
    free(this->queue_task);
}

void MyThreadPool::pthread_pool_addtask(ITask* task)
{
    if(this->thread_shutdown)
    {
        pthread_mutex_lock(&this->lock);
        while(this->queue_size == this->queue_max)
        {
            pthread_cond_wait(&this->con_productor,&this->lock);
            if(!this->thread_shutdown)
            {
                pthread_mutex_unlock(&this->lock);
                return ;
            }
        }
        this->queue_task[this->queue_front]=task;
        this->queue_front = (this->queue_front + 1)%this->queue_max;
        this->queue_size++;
        pthread_cond_signal(&this->con_customer);
        pthread_mutex_unlock(&this->lock);
    }
}

void* MyThreadPool::pthread_pool_customer(void* arg)
{
    pthread_detach(pthread_self());
    MyThreadPool* pool = (MyThreadPool*)arg;
    ITask* task;
    while(pool->thread_shutdown)
    {
        pthread_mutex_lock(&pool->lock);
        while(pool->queue_size == 0)
        {
            pthread_cond_wait(&pool->con_customer,&pool->lock);
            if(!pool->thread_shutdown || pool->thread_exitcode > 0)
            {
                pool->thread_alive--;
                pool->thread_exitcode--;
                pthread_mutex_unlock(&pool->lock);
                pthread_exit(0);
            }
        }
        task = pool->queue_task[pool->queue_rear];
        pool->queue_rear = (pool->queue_rear+1)%pool->queue_max;
        pool->queue_size--;
        pool->thread_busy++;
        pthread_mutex_unlock(&pool->lock);

        task->TaskJob();
        delete task;

        pthread_mutex_lock(&pool->lock);
        pool->thread_busy--;
        pthread_mutex_unlock(&pool->lock);
    }
    return 0;
}

void* MyThreadPool::pthread_pool_manager(void* arg)
{
    pthread_detach(pthread_self());
    MyThreadPool* pool = (MyThreadPool*)arg;
    int alive;
    int busy;
    int size;
    while(pool->thread_shutdown)
    {
        pthread_mutex_lock(&pool->lock);
        alive = pool->thread_alive;
        size = pool->queue_size;
        busy = pool->thread_busy;
        pthread_mutex_unlock(&pool->lock);

        if((size > alive - busy || busy / (float)alive *100 >= 80) && alive + pool->thread_min <= pool->thread_max)
        {
            printf("hahahahha");
            for(int i = 0;i<pool->thread_min;i++)
            {
                for(int j = 0;j<pool->thread_max;j++)
                {
                    if(pool->tids[j] == 0 || !pthread_pool_alive(pool->tids[j]))
                    {
                        pthread_mutex_lock(&pool->lock);
                        pthread_create(&pool->tids[j],NULL,pthread_pool_customer,pool);
                        pool->thread_alive++;
                        pthread_mutex_unlock(&pool->lock);
                        break;
                    }
                }
            }
        }
        if(busy*2 < alive - size && alive - pool->thread_min >= pool->thread_min)
        {
            pthread_mutex_lock(&pool->lock);
            pool->thread_exitcode = pool->thread_min;
            pthread_mutex_unlock(&pool->lock);
            for(int i = 0;i<pool->thread_min;i++)
            {
                pthread_cond_signal(&pool->con_customer);
            }
        }
        printf("manager running alive[%d] busy[%d] size[%d]\n",alive,busy,size);
        sleep(1);
    }

    pthread_exit(0);
}
bool MyThreadPool::pthread_pool_alive(pthread_t tid)
{
    pthread_kill(tid,0);
    if(errno == ESRCH)
        return false;
    return true;
}
