/**
 * @file myeManage.h
 * @brief 逻辑处理的基础类
 * @author yuming.cheng <yuming.cheng@ape-tech.com>
 * @version 
 * @date 2014-09-18
 */

#ifndef _MYE_MANAGE_H_
#define _MYE_MANAGE_H_


#include "myeUtil.h"

#ifdef __cplusplus
extern "C"
{
#endif



#define WRITETYPE 1
#define READETYPE 2

typedef struct _myeManage myeManage;

typedef struct _myeManageClass
{
    char m_name[256];  
    //构造函数 
    myeManage* (* Constructor)(void* attr);
    //析构函数
    void (* Destructor)(myeManage* me);
    
    int (*insert)(myeManage* me,int connfd,struct sockaddr_in addr);
    
    int (*process)(myeManage* me,int connfd,int type);

}myeManageClass;


int myeManage_insert(myeManage* me,int connfd,struct sockaddr_in addr);

int myeManage_process(myeManage* me,int connfd,int type);

void myeManage_delete(myeManage* me);




#ifdef __cplusplus
}
#endif

#endif 
