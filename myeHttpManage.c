#include "myeHttpManage.h"

#define MAX_CONNECT 100

struct _myeHttpManage
{
    myeManageClass* isa;
    int epollfd ;
    int m_pid;
    myeApplication* application;
    myeHttpConnect* connect;
    myeContral* contral;
};


static myeManageClass myeHttpManageClass = {
    "myeHttpManageClass",
    NULL,
    myeHttpManage_delete,
    myeHttpManage_insert,
    myeHttpManage_process
};




myeManage* myeHttpManage_new(int epollfd,int m_pid)
{
    myeHttpManage* me = (myeHttpManage*)calloc(1,sizeof(myeHttpManage));
    me->isa = &(myeHttpManageClass);
    me->epollfd = epollfd;
    me->m_pid = m_pid;
    me->application = myeApplication_new();    
    me->connect = myeHttpConnect_new_array(me->application,MAX_CONNECT);
    me->contral = myeContral_new(MAX_CONNECT);

    return (myeManage*)me;
}

int myeHttpManage_insert(myeHttpManage* me,int connfd,struct sockaddr_in addr)
{
    int index = -1;
    if(myeContral_add_socket(me->contral,connfd,&index) == -1)
    {
        return -1;
    }
    printf("max_size = [%d][%d]\n",me->contral->socket_table_size,index);
    myeHttpConnect_connect_init(me->connect+connfd,connfd,addr);
    return 0;
}

int myeHttpManage_process(myeHttpManage* me,int connfd,int type)
{
    int index = myeContral_search_socket(me->contral,connfd);
    if(index == -1)
        return -1;
    if(type == READETYPE)
    {
        enum CONNECT_CODE code = myeHttpConnect_connect_read(me->connect + index);
        int ceshi = 0;
        switch(code)
        {
        case OK:
            {
                printf("OK!!! \n")                    ;
                ceshi = 1;
                
            }
        case BUFFER_FULL:
            {
                if(ceshi == 0)
                    printf("BUFFER_FULL \n");

                enum HTTP_CODE process_ret = myeHttpConnect_process(me->connect + index);
                printf("process_ret =[%d]\n",process_ret);
                switch(process_ret)
                {
                case GET_REQUEST:
                    myeutil_epoll_modfd(me->epollfd,connfd,EPOLLOUT);
                    break;
                case CLOSED_CONNECTION:
                    myeContral_remove_socket(me->contral,connfd);
                    return -1;
                case NO_REQUEST:
                    {
                        if(code == BUFFER_FULL)
                        {
                            myeContral_remove_socket(me->contral,connfd);
                            return -1;
                        }
                        else 
                        {
                            myeutil_epoll_modfd(me->epollfd,connfd,EPOLLIN);
                        }
                    }
                default:
                    break;
                }
                break;
            }
        case IOERR:
        case CLOSED:
            {
                myeContral_remove_socket(me->contral,connfd);
                return -1;
            }
        default:
            break;
        }

    }
    else if(type == WRITETYPE)
    {
        printf("WRITETYPE\n");
        enum CONNECT_CODE code = myeHttpConnect_connect_write(me->connect + index);
        printf("code = [%d]\n",code);
        switch(code)
        {
        case TRY_AGAIN:                
            {
                myeutil_epoll_modfd( me->epollfd,connfd, EPOLLOUT ); 
                break;
            }
        case BUFFER_EMPTY:
            {
                myeutil_epoll_modfd( me->epollfd,connfd, EPOLLIN );
                break;
            }
        case IOERR:
        case CLOSED:
            {
//                myeContral_remove_socket(me->contral,connfd);
                return -1;            
            }
        default:
            break;
        }
    }
    return 0;
}

void myeHttpManage_delete(myeHttpManage* me)
{
    myeHttpConnect_delete_array(me->connect);
    myeApplication_delete(me->application);
    myeContral_delete(me->contral);
    free(me);
}
