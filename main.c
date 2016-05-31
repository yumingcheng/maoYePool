#include "myeProcessPool.h"
#include "myeEchoManage.h"
#include "myeHttpManage.h"



myeManage* myeCreate(int epollfd,int m_pid,char* name)
{
    return myeHttpManage_new(epollfd,m_pid);
}


int main(int argc,char* argv[])
{
    myeProcessPool* me = myeProcessPool_init_file("./process.conf",myeCreate);

    myeProcessPool_run(me);

    myeProcessPool_delete(me);


    return 0;
}
