#ifndef _MYE_UTIL_H_
#define _MYE_UTIL_H_

#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/epoll.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include "myeContral.h"

#define LISTEN_QUEUE_NUM 65536

#ifdef __cplusplus
extern "C"
{
#endif



    int myeutil_tcp_create(int port);

    int myeutil_setnonblocking(int fd);

    int myeutil_epoll_addfd(int epollfd,int fd);

    int myeutil_epoll_removefd(int epollfd,int fd);

    int myeutil_epoll_modfd( int epollfd, int fd, int ev );

    int myeutil_register_sig(int sig, void( handler )(int));







#ifdef __cplusplus
}
#endif

#endif 
