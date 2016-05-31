#include "myeUtil.h"
#include "myeUtil.h"


//int mye_tcp_create(int port,int (*callback)(void*,void*))
int myeutil_tcp_create(int port)
{
    int listenfd = 0; 
    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {   
        perror("socket error!");
        exit(-1);
    } 

    struct sockaddr_in servaddr;
    memset(&servaddr,0,sizeof(servaddr));
    //ipv4地址族结构赋值
    servaddr.sin_family = AF_INET ; //填写主机字节序的地址结构类型IPv4
    servaddr.sin_port = htons((uint16_t)port); // 网络字节序的端口号
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);  //存放ipv4地址，地址值为>

    if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__,"error! bind socket error");
        exit(-1);
    }

    if(listen(listenfd, LISTEN_QUEUE_NUM) !=0 )
    {   
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__,"error! socket listen error");
        exit(-1); 
    }
    return listenfd;

}

int myeutil_setnonblocking(int fd)
{
    if(fcntl(fd, F_SETFL, fcntl(fd, F_GETFL) |O_NONBLOCK) < 0)
    {
        fprintf(stderr, "%s:%d:%s\n", __FILE__, __LINE__,"error! fcntl!");
        exit(-1);
    }
    return 0;
}

int myeutil_epoll_addfd(int epollfd,int fd)
{
    struct epoll_event event ;
    memset(&event,0,sizeof(event));
    event.data.fd = fd;
    event.events = EPOLLIN | EPOLLET; //边缘触发
    epoll_ctl( epollfd, EPOLL_CTL_ADD, fd, &event );
    myeutil_setnonblocking(fd);
    return 0;
}

int myeutil_epoll_removefd(int epollfd,int fd)
{
    printf("remove [%d]\n",fd);
    epoll_ctl( epollfd, EPOLL_CTL_DEL, fd, 0 );
    close( fd );
    return 0;
}

int  myeutil_epoll_modfd( int epollfd, int fd, int ev )
{
    struct epoll_event event;
    memset(&event,0,sizeof(event));
    event.data.fd = fd;
    event.events = ev | EPOLLET;
    epoll_ctl( epollfd, EPOLL_CTL_MOD, fd, &event );
    return 0;
}                                                    

int myeutil_register_sig(int sig, void( handler )(int))
{
   struct sigaction sa;
   memset( &sa, '\0', sizeof( sa ) );
   sa.sa_handler = handler;
   sa.sa_flags |= SA_RESTART;
   sigfillset( &sa.sa_mask );
   assert( sigaction( sig, &sa, NULL ) != -1 );
   return 0;
}

