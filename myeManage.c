#include "myeManage.h"

struct _myeManage{
    myeManageClass* isa;
};


int myeManage_insert(myeManage* me,int connfd,struct sockaddr_in addr)
{
    return me->isa->insert(me,connfd,addr);
}

int myeManage_process(myeManage* me,int connfd,int type)
{
    return me->isa->process(me,connfd,type);
}

void myeManage_delete(myeManage* me)
{
    return me->isa->Destructor(me);
}




