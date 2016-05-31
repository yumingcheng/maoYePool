#include "myeProcessPool.h"

myeProcessPool* g_singleton_pool = NULL;
static const int MAX_PROCESS_NUMBER = 16; //进程池最大子进程数量
static const int USER_PER_PROCESS = 65536;//每个子进程最多能处理的客户数量
static const int MAX_EVENT_NUMBER = 10000;//epool最多能处理的事件数目
static int sig_pipefd[2]; //信号全局管道
static int EPOLL_WAIT_TIME = 5000;



/* --------------------------------------------------------------------------*/
/**
 * @brief mye_setup_sig_pipe 统一事件源之绑定信号事件
 */
/* ----------------------------------------------------------------------------*/
static void mye_setup_sig_pipe(myeProcessPool* me);

/* --------------------------------------------------------------------------*/
/**
 * @brief mye_sig_handle  注册信号的回调事件
 */
/* ----------------------------------------------------------------------------*/
static void mye_sig_handle(int sig);



/* --------------------------------------------------------------------------*/
/**
 * @brief mye_listenfd_binsearch 二分查找listenfd对应的processAllot数组坐标
 *
 * @param count_allot 数组allot_arr的长度
 * @param allot_arr
 * @param listenfd 监听套接字
 *
 * @return  成功查找到返回对应的坐标,反之返回-1
 */
/* ----------------------------------------------------------------------------*/
//static int mye_listenfd_binsearch(int count_allot,processAllot* allot_arr,int listenfd);



myeProcessPool* myeProcessPool_init(int port,int process_num,MYEMANANG manage_fun)
{
    if(g_singleton_pool != NULL)
        return g_singleton_pool;
    char p_conf_string[10240] = {0};
    sprintf(p_conf_string,"{\"tasks\":[ {\"name\":\"default\",\"host\":\"127.0.0.1\",\"port\":%d,\"process_num\":%d} ] }",port,process_num);
    return myeProcessPool_init_string(p_conf_string,manage_fun);
}

myeProcessPool* myeProcessPool_init_file(const char* p_conf_path,MYEMANANG manage_fun)
{
    if(g_singleton_pool != NULL)
        return g_singleton_pool;

    char p_conf_string[10241] = {0};
    FILE* fp = NULL;
    if ((fp = fopen(p_conf_path,"r")) == NULL)
    {
        fprintf(stderr, "%s:%d:%s", __FILE__, __LINE__,"error! 打开配置文件出错! \n");
        exit(-1);
    }
    fread(p_conf_string,10240,1,fp);
    fclose(fp);
    return myeProcessPool_init_string(p_conf_string,manage_fun);
}

myeProcessPool* myeProcessPool_init_string(const char* p_conf_string,MYEMANANG manage_fun)
{
    if(g_singleton_pool != NULL)
        return g_singleton_pool;
    myeProcessPool* me =  (myeProcessPool*)calloc(1,sizeof(myeProcessPool));
    me->m_flag_process = -1;
    me->m_flag_stop = 1;

    cJSON* root = cJSON_Parse(p_conf_string);
    printf("%s\n",p_conf_string);
    if (!root)
    {
        fprintf(stderr, "%s:%d:%s", __FILE__, __LINE__,"error! 配置文件json格式错误!\n");
        exit(-1);
    }
    cJSON * format = cJSON_GetObjectItem(root,"tasks");
    int task_num =  cJSON_GetArraySize(format);

    me->allot_arr = (processAllot*)calloc(task_num,sizeof(processAllot));
    int i = 0 ;
    int has_used = 0;
    me->sub_process_num = 0;

    for(i = 0 ; i < task_num ; i++)
    {
        cJSON* pItem = cJSON_GetArrayItem(format, i);
        if((cJSON_GetObjectItem(pItem,"name") == NULL) || (cJSON_GetObjectItem(pItem,"port") == NULL) || (cJSON_GetObjectItem(pItem,"process_num") == NULL))
        {
            fprintf(stderr, "%s:%d:%s<%d>\n", __FILE__, __LINE__,"error! 配置文件\"tasks\"内部任务缺少必要参数!",i);
            exit(-1);
        }
        me->sub_process_num += cJSON_GetObjectItem(pItem,"process_num")->valueint;         
        if(me->sub_process_num > MAX_EVENT_NUMBER)
        {
            fprintf(stderr, "%s:%d:%s<%d>\n", __FILE__, __LINE__,"error! 子进程数量不能大于",MAX_PROCESS_NUMBER);
            exit(-1);
        }
    }
    me->sub_process_arr = (myeSingleProcess*)calloc(me->sub_process_num,sizeof(myeSingleProcess));
    me->count_allot  =  task_num;    

    for(i = 0 ; i < task_num ; i++)
    {
        me->m_flag_stop = 0;
        cJSON* pItem = cJSON_GetArrayItem(format, i);
        char* name = cJSON_GetObjectItem(pItem,"name")->valuestring;    
        int port = cJSON_GetObjectItem(pItem,"port")->valueint;
        int process_num = cJSON_GetObjectItem(pItem,"process_num")->valueint;

        printf("%s/%d/%d\n",name,port,process_num);
        me->allot_arr[i].bg_index = has_used;
        me->allot_arr[i].process_num = process_num;
        me->allot_arr[i].port =  port;
        me->allot_arr[i].m_listenfd =  myeutil_tcp_create(port);
        has_used += process_num;
        int m = 0;
        for(m = 0 ; m < process_num ; m++)
        {
            me->sub_process_arr[me->allot_arr[i].bg_index + m ].m_listenfd =  me->allot_arr[i].m_listenfd;
            strncpy(me->sub_process_arr[me->allot_arr[i].bg_index + m ].name,name,255);
            me->sub_process_arr[me->allot_arr[i].bg_index + m ].manage_fun =  manage_fun;
        }
    }
    cJSON_Delete(root);

    for(i = 0 ; i < me->sub_process_num ;i ++)
    {
        int ret = socketpair( PF_UNIX, SOCK_STREAM, 0, me->sub_process_arr[i].m_pipefd );
        if(ret != 0)
        {
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__,"error! 管道创建错误!");
            exit(-1);
        }
        me->sub_process_arr[i].m_pid = fork();
        if(me->sub_process_arr[i].m_pid < 0)
        {
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__,"error! 开辟子进程出错!");
            exit(-1);
        }else if(me->sub_process_arr[i].m_pid > 0)
        {
            //父进程
            close(me->sub_process_arr[i].m_pipefd[1]);
            continue;
        }
        else 
        {
            //子进程
            close(me->sub_process_arr[i].m_pipefd[0] );
            me->m_flag_process = i;
            break;
        }
    }
    return me;
}

void myeProcessPool_run(myeProcessPool* me)
{
    if(me->m_flag_process == -1)
    {
        //说明是父进程
        myeProcessPool_run_parent(me);
    }
    else
    {
        myeProcessPool_run_child(me);
    }
}


void myeProcessPool_delete(myeProcessPool* me)
{
    printf("%s() \n",__func__);
    if(me->allot_arr)
        free(me->allot_arr);
    if(me->sub_process_arr)
        free(me->sub_process_arr);
    if(me)
        free(me);
}

void myeProcessPool_run_parent(myeProcessPool* me)
{
    mye_setup_sig_pipe(me);//信号源

    int i = 0 ;
    int j = 0 ;
    int m = 0 ;
    int n = 0 ;

    for(i = 0 ; i < me->count_allot ; i++)
    {
        myeutil_epoll_addfd(me->m_epollfd,me->allot_arr[i].m_listenfd);
//        struct epoll_event ev_set;
//        struct epoll_event ev_select;
//        ev_set.events  = EPOLLIN;
//        ev_set.data.fd = me->allot_arr[i].m_listenfd;
//        epoll_ctl(me->m_epollfd, EPOLL_CTL_ADD,me->allot_arr[i].m_listenfd, &ev_set);
//        myeutil_setnonblocking(me->allot_arr[i].m_listenfd);
    }
    for(i = 0 ; i < me-> sub_process_num ; i++)
    {
        myeutil_epoll_addfd(me->m_epollfd,me->sub_process_arr[i].m_pipefd[0]);
    }
    struct epoll_event events[ MAX_EVENT_NUMBER ];
    int num_event = 0;
    int ret = -1;
    int flag = 0;
    int def_int = 1;
    int def_mao = 0;
    int count  = 0;
    while( ! me->m_flag_stop )
    {
        num_event = epoll_wait(me->m_epollfd, events,MAX_EVENT_NUMBER, EPOLL_WAIT_TIME );
        count += num_event;
        printf("count [%d]\n",count);
        if(( num_event < 0 ) && ( errno != EINTR ))
        {
            fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__,"error! epoll error!");
            break;
        }

        for(i = 0 ; i < num_event ; i++)
        {
            int sockfd = events[i].data.fd;
            if(( sockfd == sig_pipefd[0] ) && ( events[i].events & EPOLLIN ))
            {
                char signals[1024];
                ret = recv( sig_pipefd[0], signals, sizeof( signals ), 0 );
                if( ret <= 0 )
                {
                    continue;
                }
                else
                {
                    for( j = 0; j < ret; j++ )
                    {
                        printf("ret = [%d]\n",ret);
                        switch( signals[j] )
                        {
                        case SIGCHLD:
                            {
                                pid_t pid;
                                int stat;
                                while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
                                {
                                   for( m = 0; m < me->sub_process_num; m++ )
                                   {
                                       if( me->sub_process_arr[m].m_pid == pid )
                                       {
                                           printf( "child %d join\n", m );
                                           close( me->sub_process_arr[m].m_pipefd[0] );
                                           me->sub_process_arr[m].m_pid = -1;
                                       }
                                       printf("m = %d\n",m);
                                   }
                                }
                                fflush(stdin);
                                printf("adfasdfadf");

                                me->m_flag_stop = 1;
                                for( n = 0; n < me->sub_process_num; n++ )
                                {
                                    if( me->sub_process_arr[n].m_pid != -1 )
                                    {
                                        me->m_flag_stop = 0;
                                    }
                                }
                                printf("父 ！！！[%d] \n",me->m_flag_stop);
                                break;
                            }
                        case SIGTERM:
                        case SIGINT:
                            {
                                printf( "kill all the clild now\n" );
                                for( m = 0; m < me->sub_process_num; m++)
                                {

                                    int pid = me->sub_process_arr[m].m_pid;                                
                                    printf("pid =[%d]\n",pid);
                                    if( pid != -1 )
                                    {
                                        kill( pid, SIGTERM );
                                    }
                                }
                                break;
                            }
                        default:
                            {
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                flag = 0;
                for(j = 0 ; j < me->count_allot ;j++)
                {
                    if( me->allot_arr[j].m_listenfd == sockfd )
                    {
                        def_mao++;
                        flag = 1;
                        m  =  me->allot_arr[j].process_index;
                        do
                        {
                            if( me->sub_process_arr[m].m_pid != -1 )
                            {
                                break;
                            }
                            m = (m + 1) % (me -> allot_arr[j].process_num);

                        }while( m != me->allot_arr[j].process_index );

                        if(me->sub_process_arr[m].m_pid == -1 )
                        {
                            me->m_flag_stop = 1;
                            break;
                        }
                        me -> allot_arr[i].process_index = ( m + 1) % ( me -> allot_arr[j].process_num );
                        send( me->sub_process_arr[ m +  me -> allot_arr[j].bg_index ].m_pipefd[0],(char*)&sockfd,sizeof(sockfd),0);
                        printf( "send request to child [%d][%d][%d]\n", j,def_int,def_mao);
                        def_int++;
                        break;                        
                    }
                }
                
                if((events[i].events & EPOLLIN) && (flag == 0))
                {
                    for(m = 0 ; m < me->sub_process_num ; m++ )
                    {
                        if( sockfd == me->sub_process_arr[m].m_pipefd[0] )
                        {
                                //自进程与父进程的通信
                        }
                    }
                }
            }
        }
    }
    for(i = 0 ; i < me->count_allot ; i++)
    {
        myeutil_epoll_removefd(me->m_epollfd,me->allot_arr[i].m_listenfd);
    }
    for(i = 0 ; i < me->sub_process_num; i++)
    {
        myeutil_epoll_removefd(me->m_epollfd,me->sub_process_arr[i].m_pipefd[0]);
    }
    close(me->m_epollfd);
}

void myeProcessPool_run_child(myeProcessPool* me)
{
    mye_setup_sig_pipe(me);
    myeSingleProcess* sig = me -> sub_process_arr + me -> m_flag_process;

    int pipefd = sig->m_pipefd[1];

    myeutil_epoll_addfd(me->m_epollfd,pipefd); //将与附近程通讯的管道加入epoll监听
    struct epoll_event events[ MAX_EVENT_NUMBER ];
    myeManage* manage = (sig->manage_fun)(me->m_epollfd,sig->m_pid,sig->name);
    int num_event = 0;
    int ret = -1;
    int i = 0;
    int def_int = 1;
    int def_mao = 1;
    while( ! me->m_flag_stop )
    {
        num_event = epoll_wait( me->m_epollfd, events, MAX_EVENT_NUMBER, EPOLL_WAIT_TIME );
        if(( num_event < 0 ) && ( errno != EINTR ))
        {
            fprintf(stderr, "pid=[%d] %s:%d:%s\n",sig->m_pid,__FILE__, __LINE__,"error! epoll error!");
            break;
        }

        for(i = 0 ; i < num_event ; i++)
        {
            int sockfd = events[i].data.fd;
            if( ( sockfd == pipefd ) && ( events[i].events & EPOLLIN ) )
            {
                printf("def_int [%d]\n",def_int);
                def_int++;
                int client = 0;
                ret = recv( sockfd, ( char* )&client, sizeof( client ), 0 );
                if( ( ( ret < 0 ) && ( errno != EAGAIN ) ) || ret == 0 ) 
                {
                    printf("ret error! %d",ret);
                    continue;
                }
                else
                {
                    int connfd = -1;
                    do
                    {
                        struct sockaddr_in client_address;
                        socklen_t addr_size = sizeof( client_address );
                        connfd = accept( sig->m_listenfd, ( struct sockaddr* )&client_address, &addr_size );
                        printf("accept %d [%d][%d]\n",me -> m_flag_process,connfd,def_mao);
                        if ( connfd < 0 )
                        {
                            printf( "errno is: %d\n", errno );
                            break;
                        }
                        def_mao++;
                        myeutil_epoll_addfd(me->m_epollfd,connfd);
                        if(myeManage_insert(manage,connfd,client_address) == -1)
                        {
                            myeutil_epoll_removefd(me->m_epollfd,connfd);
                        }

                    }while(connfd > 0);
                }
            }
            else if( ( sockfd == sig_pipefd[0] ) && ( events[i].events & EPOLLIN ) )
            {
                char signals[1024];
                ret = recv( sig_pipefd[0], signals, sizeof( signals ), 0 );
                if( ret <= 0 )
                {
                    continue;
                }
                else
                {
                    for( i = 0; i < ret; ++i )
                    {
                        switch( signals[i] )
                        {
                        case SIGCHLD:
                            {
                                pid_t pid;
                                int stat;
                                while ( ( pid = waitpid( -1, &stat, WNOHANG ) ) > 0 )
                                {
                                    continue;
                                }
                                break;
                            }
                        case SIGTERM:
                        case SIGINT:
                            {
                                printf("[%d] stop  \n",me->m_flag_process);
                                me->m_flag_stop = 1;
                                break;
                            }
                        default:
                            {
                                break;
                            }
                        }
                    }
                }
            }
            else if(events[i].events & EPOLLIN)
            {
                if(myeManage_process(manage,sockfd,READETYPE) == -1)
                {
                    myeutil_epoll_removefd(me->m_epollfd,sockfd);
                    continue;
                }
            }
            else if(events[i].events & EPOLLOUT)
            {
                if(myeManage_process(manage,sockfd,WRITETYPE) == -1)
                {
                    myeutil_epoll_removefd(me->m_epollfd,sockfd);
                    continue;
                }
            }
            else
            {
                continue;
            }
        }
    }
    myeManage_delete(manage);
    close( pipefd );
    close( me->m_epollfd );
}

static void mye_sig_handle(int sig)
{
    int save_errno = errno;
    int msg = sig;
    printf("%s() %d [sig]\n",__func__,__LINE__);
    send( sig_pipefd[1], ( char* )&msg, 1, 0 );
    errno = save_errno;
}

static void mye_setup_sig_pipe(myeProcessPool* me)
{
    me->m_epollfd = epoll_create(5);
    if(me->m_epollfd == -1)
    {
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__,"error! epoll_create 失败!");
        exit(-1);
    }
    int ret = socketpair( PF_UNIX, SOCK_STREAM, 0,sig_pipefd);
    if(ret != 0)
    {
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__,"error! 管道创建错误!");
        exit(-1);
    }

    myeutil_setnonblocking(sig_pipefd[1]);
    myeutil_epoll_addfd(me->m_epollfd,sig_pipefd[0]);
    myeutil_register_sig( SIGCHLD, mye_sig_handle);  
    myeutil_register_sig( SIGTERM, mye_sig_handle);
    myeutil_register_sig( SIGINT,  mye_sig_handle);
    /*在我们的环境中当网络触发broken pipe (一般情况是write的时候，没有write完毕， 接受端异常断开了)， 系统默认的行为是直接退出。在我们的程序中一般都要在启动的时候加上 signal(SIGPIPE, SIG_IGN); 来强制忽略这种错误*/
    myeutil_register_sig( SIGPIPE, SIG_IGN );

}





