#ifndef LOG_H
#define LOG_H
#include <stdio.h>
#include <iostream>
#include <string>
#include <stdarg.h>
#include <pthread.h>
#include "block_queue.h"
class Log{
public:
    static Log *get_instance(){
        static Log instance;
        return &instance;
    }
    static void *flush_log_thread(void *args){
        FILE* fp = fopen("test.txt", "a");
        fflush(fp);
        fputs("flush_log_thread\n", fp);
        std::cout<<"flush log thread"<<std::endl;
        Log::get_instance()->async_write_log();
    }
    void write_log(int level, const char *format, ...);
    bool init(const char* filename, int close_log, int log_buf_size = 8192, int split_lines = 5000000, int max_queue_size=0);
    void flush(void);
private:
    Log();
    virtual ~Log();
    void async_write_log(){
        std::string single_log;
        std::cout<<"async_write_log"<<std::endl;
        FILE* fp = fopen("test.txt", "a");
        fputs("async_write_log\n", fp);
        std::cout<<"File Address: "<<m_fp<<std::endl;
        std::cout<<"File Name:"<<FileName<<std::endl;
        while(!m_close_log || (m_close_log && !m_log_queue->empty())){
            m_log_queue->pop(single_log);
            m_mutex.lock();
            fputs(single_log.c_str(), m_fp);
            m_mutex.unlock();
        }
        fflush(m_fp);
    }
private:
    pthread_t tid;
    char dir_name[128]; //日志的路径名
    char log_name[128]; //log文件名
    int m_split_lines; //日志最大行数
    int m_log_buf_size; //日志缓冲区的大小是多少
    long long m_count;  //日志行记录数
    char FileName[256];
    int m_today;        //日志的天数[1,31]
    int m_mon;          //日志的月份
    int m_year;         //日志的年份
    FILE *m_fp;         //Log文件对应的指针
    char *m_buf;        //?
    block_queue<std::string> *m_log_queue; //阻塞队列
    bool m_is_async;    //是否同步标志位
    std::mutex m_mutex;
    int m_close_log;    //关闭日志
};

#define LOG_DEBUG(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(0, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_INFO(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(1, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_WARN(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(2, format, ##__VA_ARGS__); Log::get_instance()->flush();}
#define LOG_ERROR(format, ...) if(0 == m_close_log) {Log::get_instance()->write_log(3, format, ##__VA_ARGS__); Log::get_instance()->flush();}

#endif