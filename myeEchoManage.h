#ifndef _MYE_ECHO_MANAGE_H_
#define _MYE_ECHO_MANAGE_H_


#include "myeManage.h"

#ifdef __cplusplus
extern "C"
{
#endif


    typedef struct _myeEchoManage myeEchoManage;

    myeManage* myeEchoManage_new(int epollfd,int m_pid);

    int myeEchoManage_insert(myeEchoManage* me,int connfd,struct sockaddr_in addr);

    int myeEchoManage_process(myeEchoManage* me,int connfd,int type);

//    int myeEchoManage_process_read(myeEchoManage* me,int connfd);
//
//    int myeEchoManage_process_write(myeEchoManage* me,int connfd);

    void myeEchoManage_delete(myeEchoManage* me);




#ifdef __cplusplus
}
#endif


#endif
