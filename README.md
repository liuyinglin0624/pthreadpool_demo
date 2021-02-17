# pthreadpool_demo
线程池的实现
#### 语言
- C++
#### 平台
- Linux
#### 功能
- 采用生产者与消费者模型，实现线程池动态的扩容与缩减
#### 对外接口
- 线程池的创建：pthread_pool_create(int max,int min,int queue_max);
- 投递任务：pthread_pool_addtask(ITask* task);
- 线程池的销毁：pthread_pool_destroy();
- 任务基类：class ITask;
#### 细节
- main.cpp是线程池的封装测试
