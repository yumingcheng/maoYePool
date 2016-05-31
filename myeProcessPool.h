/**
 * @file myeProcessPool.h
 * @brief 
 * @author yuming.cheng <yuming.cheng@ape-tech.com>
 * @version 
 * @date 2014-09-15
 */

#ifndef _MYE_PROCESS_POOL_H_
#define _MYE_PROCESS_POOL_H_

#include "myeManage.h"
#include "cJSON.h"

#ifdef __cplusplus
extern "C"
{
#endif

    typedef myeManage* (* MYEMANANG)(int epollfd,int m_pid,char* name);

    typedef struct  _processAllot
    {
        int m_listenfd;
        int bg_index;
        int process_num;
        int port;
        int process_index;//用作轮询链接

    }processAllot;


    typedef struct _myeSingleProcess
    {
        pid_t m_pid;
        int m_pipefd[2];
        int m_listenfd;
        MYEMANANG manage_fun; 
        char name[256];//任务名称

    }myeSingleProcess;


    typedef struct _myeProcessPool
    {

        int m_flag_process;//标记是父进程还是子进程
        int m_flag_stop;//标识是否结束主进程
        int m_epollfd;
        processAllot* allot_arr; //任务管理
        int count_allot; //任务数目
        myeSingleProcess* sub_process_arr; //子进程管理
        int sub_process_num;//子进程数目

    }myeProcessPool;


    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeProcessPool_init 进程池初始化(监听一个端口)
     *
     * @param port  监听套接字绑定的主机端口号
     * @param num_process   开辟的进程数目
     * @param manage_fun 回调函数,产生处理对象句柄 
     *
     * @return 进程池句柄
     */
    /* ----------------------------------------------------------------------------*/
    myeProcessPool* myeProcessPool_init(int port,int process_num,MYEMANANG manage_fun);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeProcessPool_init_file 进程池初始化,读取配置文件
     *
     * @param p_conf_path   配置文件路径
     * @param manage_fun 回调函数,产生处理对象句柄 
     *
     * @return  进程池句柄
     */
    /* ----------------------------------------------------------------------------*/
    myeProcessPool* myeProcessPool_init_file(const char* p_conf_path,MYEMANANG manage_fun);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeProcessPool_init_string 进程池初始化,更具string数据配置
     *
     * @param p_conf_string 配置具体数据
     * @param manage_fun 回调函数,产生处理对象句柄 
     *
     * @return 进程池句柄
     */
    /* ----------------------------------------------------------------------------*/
    myeProcessPool* myeProcessPool_init_string(const char* p_conf_string,MYEMANANG manage_fun);



    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeProcessPool_run 进程池运行
     *
     * @param me
     */
    /* ----------------------------------------------------------------------------*/
    void myeProcessPool_run(myeProcessPool* me);


    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeProcessPool_run_parent 内部运行接口,父进程运行代码
     */
    /* ----------------------------------------------------------------------------*/
    void myeProcessPool_run_parent(myeProcessPool* me);  


    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeProcessPool_run_child 内部运行接口,子进程运行代码
     */
    /* ----------------------------------------------------------------------------*/
    void myeProcessPool_run_child(myeProcessPool* me);

    /* --------------------------------------------------------------------------*/
    /**
     * @brief myeProcessPool_delete 释放
     *
     * @param me
     */
    /* ----------------------------------------------------------------------------*/
    void myeProcessPool_delete(myeProcessPool* me);




#ifdef __cplusplus
}
#endif


#endif
