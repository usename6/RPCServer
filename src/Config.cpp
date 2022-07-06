#include "Config.h"
#include <string>
#include <iostream>
Config::Config(){
    IP = "127.0.0.1";
    //DataBase IP
    DBIP = "localhost";
    //DataBase Username
    username = "root";
    //DataBase Password
    password = "root";
    //DataBase Name
    DBName = "yourdb";
    //DataBase Port
    DBPORT = 3306;
    //端口号,默认9006
    PORT = 9006;

    //日志写入方式，默认同步
    LOGWrite = 0;

    //触发组合模式,默认listenfd LT + connfd LT
    TRIGMode = 0;

    //listenfd触发模式，默认LT
    LISTENTrigmode = 0;

    //connfd触发模式，默认LT
    CONNTrigmode = 0;

    //优雅关闭链接，默认不使用
    OPT_LINGER = 0;

    //数据库连接池数量,默认8
    sql_num = 8;

    //线程池内的线程数量,默认8
    thread_num = 8;

    //关闭日志,默认不关闭
    close_log = 0;

    //并发模型,默认是proactor
    actor_model = 0;
}

void Config::parse_arg(int argc, char*argv[]){
    int opt;
    //i --> IP 
    //d --> database name
    //x --> database ip
    //u --> databse username
    //w --> database passwd
    //r --> databse port
    const char *str = "i:d:x:u:w:r:p:l:m:o:s:t:c:a:";
    //const char *str = "p:l:m:o:s:t:c:a:";
    while ((opt = getopt(argc, argv, str)) != -1)
    {
        std::cout<<"opt = "<<opt<<std::endl;
        switch (opt)
        {
        case 'i':{
            IP = optarg;
            break;
        }
        case 'd':{
            DBName = optarg;
            break;
        }
        case 'x':{
            DBIP = optarg;
            break;
        }
        case 'u':{
            username = optarg;
            break;
        }
        case 'w':{
            password = optarg;
            break;
        }
        case 'r':{
            DBPORT = atoi(optarg);
            break;
        }
        case 'p':{
            PORT = atoi(optarg);
            break;
        }
        case 'l':{
            LOGWrite = atoi(optarg);
            break;
        }
        case 'm':{
            TRIGMode = atoi(optarg);
            break;
        }
        case 'o':{
            OPT_LINGER = atoi(optarg);
            break;
        }
        case 's':{
            sql_num = atoi(optarg);
            break;
        }
        case 't':{
            thread_num = atoi(optarg);
            break;
        }
        case 'c':{
            close_log = atoi(optarg);
            break;
        }
        case 'a':{
            actor_model = atoi(optarg);
            break;
        }
        default:
            break;
        }
    }
}