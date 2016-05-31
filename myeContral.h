#ifndef _MYE_CONTRAL_H_
#define _MYE_CONTRAL_H_

#include <stdio.h>
#include <stdlib.h>

#define MAX_SOCKET 65536
#define BUF_SIZE 102400

#ifdef __cplusplus
extern "C"
{
#endif

    typedef struct _myeContral
    {
        int*  socket_table;
        int socket_table_size;
        int max_size ;

    }myeContral;


    myeContral* myeContral_new(int max_size);

    int myeContral_add_socket(myeContral* me,int sockfd,int* index);

    int myeContral_search_socket(myeContral* me,int sockfd);

    int myeContral_remove_socket(myeContral* me,int sockfd);

    void myeContral_delete(myeContral* me);





#ifdef __cplusplus
}
#endif

#endif

