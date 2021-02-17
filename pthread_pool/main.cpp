#include <iostream>
#include<mythreadpool.h>
using namespace std;

class PrintTask:public ITask
{
public:
     void TaskJob()
     {
         printf("running.....\n");

     }
};

int main()
{
    MyThreadPool pool;
    pool.pthread_pool_create(100,10,100);

    for(int i = 0;i<60;i++)
    {
        ITask* task = new PrintTask;
        pool.pthread_pool_addtask(task);
    }

    while(1);

    cout << "Hello World!" << endl;
    return 0;
}
