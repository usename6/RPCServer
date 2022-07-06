#include <string.h>
#include <time.h>
#include <sys/time.h>
#include <stdarg.h>
#include "Log.h"
#include <time.h>
#include <pthread.h>

Log::Log(){
    m_count = 0;
    m_is_async = false;  //不同步
}

Log::~Log(){
    //检查文件指针是否关闭
    m_close_log = 1;
    pthread_join(tid, NULL);
    std::cout<<"Bye Bye"<<std::endl;
    if(m_fp != NULL){
        fclose(m_fp);
    }
}

bool Log::init(const char *file_name, int close_log, int log_buf_size, int split_lines, int max_queue_size){
    m_close_log = close_log;
    m_log_buf_size = log_buf_size;
    m_buf = new char[m_log_buf_size];
    memset(m_buf, '\0', m_log_buf_size);
    m_split_lines = split_lines;

    time_t t = time(NULL);
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;

    const char* p = strrchr(file_name, '/');
    char log_full_name[256] = {0};
    //当前目录
    if(p == NULL){
        //将my_tm的信息和filename联合起来，格式
        snprintf(log_full_name, 255, "%d_%02d_%02d_%s", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, file_name);
    }
    //特定目录的情况
    else{
        //从p开始将文件的名字赋值给log_name
        strcpy(log_name, p+1);
        //p代表的是最后一次出现/的指针的位置，和file_name的头指针位置相减，得到要复制多少个字符
        strncpy(dir_name, file_name, p - file_name + 1);
        //格式化输入道log_full_name里
        snprintf(log_full_name, 255, "%s%d_%02d_%02d_%s", dir_name, my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday, log_name);
    }
    std::cout<<"log_full_name = "<<log_full_name<<std::endl;
    m_today = my_tm.tm_mday;    //初始化当前日志是第几天的
    m_mon = my_tm.tm_mon;       //初始化当前日志是第几月的
    m_year = my_tm.tm_year;     //初始化当前日志是第几年的
    m_fp = fopen(log_full_name, "a");
    
    if(m_fp == NULL){
        std::cout<<"init is failed!"<<std::endl;
        return false;
    }
    fputs("usename6 is here!\n", m_fp);
    strcpy(FileName, log_full_name);
    std::cout<<"init is success!"<<std::endl;
    std::cout<<"File Address: "<<m_fp<<std::endl;
    //如果设置了max_queue_size，则表示异步处理
    if(max_queue_size >= 1){
        //异步标志位打开
        m_is_async = true;
        //创建阻塞队列
        m_log_queue = new block_queue<std::string>(max_queue_size);
        //flush_log_thread为回调函数,这里表示创建线程异步写日志
        pthread_create(&tid, NULL, flush_log_thread, NULL);
    }
    return true;
}

void Log::write_log(int level, const char* format, ...){
    //timeval --> (tv_sec, tv_usec)
    // s , ms
    struct timeval now = {0, 0};
    gettimeofday(&now, NULL);
    time_t t = now.tv_sec;   //t s
    //localtime是 把从1970-1-1零点零分到当前时间系统所偏移的秒数时间转换为本地时间
    //int tm_sec;           /* Seconds. [0-60] (1 leap second) */
    //int tm_min;           /* Minutes. [0-59] */
    //int tm_hour;          /* Hours.   [0-23] */
    //int tm_mday;          /* Day.     [1-31] */
    //int tm_mon;           /* Month.   [0-11] */
    //int tm_year;          /* Year - 1900.  */
    //int tm_wday;          /* Day of week. [0-6] */
    //int tm_yday;          /* Days in year.[0-365] */
    //int tm_isdst;         /* DST.     [-1/0/1]*/
    struct tm *sys_tm = localtime(&t);
    struct tm my_tm = *sys_tm;
    //存储消息的前缀
    char s[16] = {0};
    //解析消息的前缀
    switch (level)
    {
    case 0:
        strcpy(s, "[debug]:");
        break;
    
    case 1:
        strcpy(s, "[info]:");
        break;

    case 2:
        strcpy(s, "[warn]:");
        break;

    case 3:
        strcpy(s, "[erro]:");
        break;
    
    default:
        strcpy(s, "[info]:");
        break;
    }
    std::cout<<"level: "<< s<<std::endl;
    m_mutex.lock();
    //行数++
    m_count++;
    //检查当前日期和现在的日期是否一样,不一样重新创建日志文件
    //检查是否行数是否超过日志的最大行数
    if(m_today != my_tm.tm_mday || m_count % m_split_lines == 0){
        char new_log[256] = {0};
        std::cout<<"start"<<std::endl;
        //fflush()会强迫将缓冲区内的数据写回参数stream 指定的文件中
        fflush(m_fp);
        fclose(m_fp);
        char tail[16] = {0};
        //把新的日期写入到tail里
        snprintf(tail, 16, "%d_%02d_%02d_", my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday);
        std::cout<<"tail :"<<tail<<std::endl;
        //如果是日期不一样
        if(m_today != my_tm.tm_mday){
            snprintf(new_log, 255, "%s%s%s", dir_name, tail, log_name);
            m_today = my_tm.tm_mday;
            m_count = 0;
        }
        //如果是行数超过了
        else{
            snprintf(new_log, 255, "%s%s%s.%lld", dir_name, tail, log_name, m_count / m_split_lines);
        }
        std::cout<<"new_log = "<<new_log<<std::endl;
        m_fp = fopen(new_log, "a");
    }
    m_mutex.unlock();
    //可变参数的列表,放入到va_list里
    std::cout<<"create m_fp over!"<<std::endl;
    va_list valst;
    va_start(valst, format);

    std::string log_str;
    m_mutex.lock();
    //m_buf --> 一行里的信息
    int n = snprintf(m_buf, 48, "%d-%02d-%02d %02d:%02d:%02d.%06ld %s ",
                     my_tm.tm_year + 1900, my_tm.tm_mon + 1, my_tm.tm_mday,
                     my_tm.tm_hour, my_tm.tm_min, my_tm.tm_sec, now.tv_usec, s);
    
    int m = vsnprintf(m_buf + n, m_log_buf_size - 1, format, valst);
    m_buf[n + m] = '\n';       //因为只写一行
    m_buf[n + m + 1] = '\0';   //字符串的终止符
    log_str = m_buf;           //结束
    std::cout<<"log_str = "<<log_str<<std::endl;
    m_mutex.unlock();
    if (m_is_async && !m_log_queue->full()){
        //异步操作
        std::cout<<"async is start!"<<std::endl;
        m_log_queue->push(log_str);
    }
    else{
        //同步操作
        m_mutex.lock();
        fputs(log_str.c_str(), m_fp);
        m_mutex.unlock();
    }
    va_end(valst);
}

void Log::flush(void){
    m_mutex.lock();
    std::cout<<"flush start"<<std::endl;
    fflush(m_fp);
    std::cout<<"flush over"<<std::endl;
    m_mutex.unlock();
}