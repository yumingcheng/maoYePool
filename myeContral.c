
#include "myeContral.h"

static int get_socket_id_hash(myeContral* me,int sockfd)
{
    int hash = 0;
    hash = sockfd % (me->max_size);
    return hash;
}

static int search_socket_id(myeContral* me,int sockfd)
{
    unsigned int hash = get_socket_id_hash(me,sockfd);
    while (1)
    {
        if (me->socket_table[hash] == -1)
            return -1;
        if (me->socket_table[hash] == sockfd)
            return hash;
        hash = (hash + 1) % (me->max_size);
    }
    return -1;
}

static int add_socket_id(myeContral* me,int sockfd,int* index)
{
    unsigned int hash = 0; 
    //单词hash编码
    hash = get_socket_id_hash(me,sockfd);
    //将单词hash编码保存,如果hash位置存在，放入最进一个空闲的地方
    while (me->socket_table[hash] != -1)
        hash = (hash + 1) % (me->max_size) ;
    me->socket_table[hash] = sockfd;
    me->socket_table_size += 1;
    *index = hash;
    return me->socket_table_size;
}

myeContral* myeContral_new(int max_size)
{
    myeContral* me = (myeContral*)calloc(1,sizeof(myeContral));
    me->socket_table = (int*)calloc(max_size,sizeof(int));
    me->max_size = max_size;
    int i = 0;
    for(i = 0 ; i < max_size ; i++)
    {
        me->socket_table[i] = -1;
    }
    return me;
}

int myeContral_add_socket(myeContral* me,int sockfd,int* index)
{
    if(search_socket_id(me,sockfd) == -1)
    {
        if ( me->socket_table_size > me->max_size * 0.7)
        {
            //逻辑是不允许继续链接,实际应该不可能出现这种情况
            return -1;
        }
        add_socket_id(me,sockfd,index);
        return 0;
    }
    else
    {
        //此时证明用户登录两次,必须将前一次的链接关闭
        return -1;
    }
    return 0;
}

int myeContral_search_socket(myeContral* me,int sockfd)
{
    return search_socket_id(me,sockfd) ;
}

int myeContral_remove_socket(myeContral* me,int sockfd)
{
    int index = search_socket_id(me,sockfd) ;
    if(index == -1)
        return 0;
    else
    {
        me->socket_table[index] = -1;
    }
    me->socket_table_size -= 1;
    return 0;
}

void myeContral_delete(myeContral* me)
{
    free(me->socket_table);
    if(me)
        free(me);
}
