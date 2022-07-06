#ifndef CONFIG_H
#define CONFIG_H
#include <string>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <unistd.h>
class Config
{
public:
    Config();
    ~Config(){};

    void parse_arg(int argc, char*argv[]);
    std::string IP;
    //Database IP地址
    std::string DBIP;
    //DataBase usename
    std::string username;
    //DataBase password
    std::string password;
    //DataBase Name
    std::string DBName;
    //DataBase Port
    int DBPORT;
    //端口号
    int PORT;
    //日志写入方式
    int LOGWrite;
    //触发组合模式
    int TRIGMode;
    //listenfd触发模式
    int LISTENTrigmode;
    //connfd触发模式
    int CONNTrigmode;
    //优雅关闭链接
    int OPT_LINGER;
    //数据库连接池数量
    int sql_num;
    //线程池内的线程数量
    int thread_num;
    //是否关闭日志
    int close_log;
    //并发模型选择
    int actor_model;
};

#endif