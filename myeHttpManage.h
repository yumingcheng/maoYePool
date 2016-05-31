#ifndef _MYE_HTTP_MANAGE_H_
#define _MYE_HTTP_MANAGE_H_


#include "myeManage.h"
#include "../maoYeHTTP/myeHttpConnect.h"


#ifdef __cplusplus
extern "C"
{
#endif


    typedef struct _myeHttpManage myeHttpManage;

    myeManage* myeHttpManage_new(int epollfd,int m_pid);

    int myeHttpManage_insert(myeHttpManage* me,int connfd,struct sockaddr_in addr);

    int myeHttpManage_process(myeHttpManage* me,int connfd,int type);

//    int myeHttpManage_process_read(myeHttpManage* me,int connfd);
//
//    int myeHttpManage_process_write(myeHttpManage* me,int connfd);

    void myeHttpManage_delete(myeHttpManage* me);




#ifdef __cplusplus
}
#endif


#endif
