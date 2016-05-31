#include "myeEchoManage.h"

struct _myeEchoManage
{
    myeManageClass* isa;
    int epollfd ;
    int m_pid;
};


static myeManageClass myeEchoManageClass = {
    "myeEchoManageClass",
    NULL,
    myeEchoManage_delete,
    myeEchoManage_insert,
    myeEchoManage_process
};


myeManage* myeEchoManage_new(int epollfd,int m_pid)
{
    myeEchoManage* me = (myeEchoManage*)calloc(1,sizeof(myeEchoManage));
    me->isa = &(myeEchoManageClass);
    me->epollfd = epollfd;
    me->m_pid = m_pid;
    printf("%s() \n",__func__);

    return (myeManage*)me;
}

int myeEchoManage_insert(myeEchoManage* me,int connfd,struct sockaddr_in addr)    
{
    return 0;
}

int myeEchoManage_process(myeEchoManage* me,int connfd,int type)
{
    printf("%s() [%d][%d]\n",__func__,connfd,type);
    int bytes_read = 0;
    int bytes_write = 0;
    char temp[1024] = {0};    
    char send_str[1024] = "中国电信万岁";
    if(type == READETYPE)
    {
        memset(temp,0,1024);
        while( 1 )
        {
            bytes_read = recv( connfd,temp+bytes_read,1023, 0 );
            printf("read in %d bytes from socket %d with content: [%s]\n", bytes_read,connfd,temp);
            if ( bytes_read == -1 )
            {
                if(errno == EAGAIN||errno == EWOULDBLOCK||errno == EINTR)
                {
                    break;
                }
            }
            else if ( bytes_read == 0 )
            {
                return -1;
            }
        }
        printf("recv [%s]\n",temp);
        myeutil_epoll_modfd(me->epollfd ,connfd,EPOLLOUT);

    }
    else if(type == WRITETYPE)
    {
        bytes_write = send( connfd,send_str,1024, 0 );
        if ( bytes_write == -1 )
        {
            if( errno == EAGAIN || errno == EWOULDBLOCK )
            {
                    bytes_write = send( connfd,send_str,1024, 0 );
            }
            return -1;
        }
        else if ( bytes_write == 0 )
        {   
            return -1;
        }

    }
    return 0;
}

void myeEchoManage_delete(myeEchoManage* me)
{
}


