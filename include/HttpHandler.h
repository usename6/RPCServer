#ifndef HTTP_HANDLER_H_
#define HTTP_HANDLER_H_
#include "reactor.h"
#include <string>
#include <sstream>
#include <unordered_map>
#include <vector>
#include <cstring>
#include <cstdio>
#include <cstdlib>
#include <algorithm>
#include <unordered_set>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>
#include "DataBase_Pool.h"
#include "Config.h"
#include <iostream>
#include "RpcApplication.hpp"
#include "RpcControl.hpp"
#include "User.pb.h"
#include "RedisUtils.h"
#include "RpcChannel.hpp"
//#include <mysql.h>
typedef Singleton<Connection_Pool> SPool;
#define connPool SPool::getInstance()
typedef Singleton<Config> SConfig;
#define config SConfig::getInstance()
typedef Singleton<reactor::Reactor> Sreactor;
#define g_reactor Sreactor::getInstance()
class HTTPHandler : public reactor::EventHandler{
public:
    enum METHOD{
        GET = 0,
        POST,
        HEAD,
        PUT,
        DELETE,
        TRACE,
        OPTIONS,
        CONNECT,
        PATH
    };
    enum CHECK_STATE{
        CHECK_STATE_REQUESTLINE = 0,
        CHECK_STATE_HEADER,
        CHECK_STATE_CONTENT
    };
    enum HTTPCODE{
        NO_REQUEST = 0,
        GET_REQUEST,
        BAD_REQUEST,
        NO_RESOURCE,
        FORBIDDEN_REQUEST,
        FILE_REQUEST,
        INTERNAL_ERROR,
        CLOSED_CONNECTION
    };
    enum LINE_STATUS{
        LINE_OK = 0,
        LINE_BAD,
        LINE_OPEN
    };
public:
    HTTPHandler(){
        std::cout<<"create HTTPHandler start!"<<std::endl;
        redisUtils = new RedisUtils();
        init("/home/reactor_demo/root", 0, config->username, config->password, config->DBName);
        //initmysql_result();
    }
    ~HTTPHandler(){}
    HTTPHandler(reactor::handler_t handle):m_handle(handle){}
public:
    void initmysql_result();
    void HandleRead() override{
        //std::cout<<"Handle Read is start!"<<std::endl;
        //如果读入已经超过缓冲区，退出
        if(m_read_idx>=READ_BUFFER_SIZE){
            readok = false;
            if (g_reactor->RegisterHandler(this, reactor::kWriteEvent) != 0){
                printf("error: register handler failed\n");
                //delete handler;
            }
            return;
        }
        int bytes_read=0;
        while(true){
            bytes_read = recv(m_handle, m_read_buf+m_read_idx, READ_BUFFER_SIZE-m_read_idx, 0);
            if(bytes_read == -1){
                if(errno == EAGAIN || errno == EWOULDBLOCK){
                    break;
                }
                readok = false;
                if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0){
                    printf("error: register handler failed\n");
                    //delete handler;
                }
                return;
            }
            else if(bytes_read == 0){
                readok = false;
                if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0){
                    printf("error: register handler failed\n");
                    //delete handler;
                }
                return;
            }
            m_read_idx += bytes_read;
        }
        if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0){
            printf("error: register handler failed\n");
            //delete handler;
        }
        readok = true;
        process();
        return;
    }
    void HandleWrite() override{
        int temp = 0;
        int newadd = 0;
        if(bytes_to_send == 0){
            init();
            if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0){
                printf("error: register handler failed\n");
                //delete handler;
            }
            writeok = true;
            return;
        }
        while(true){
            temp = writev(m_handle, m_iv, m_iv_count);
            if(temp > 0){
                bytes_have_send += temp;
                bytes_to_send -= temp;
                //第二段文件报文的位移在哪
                newadd = bytes_have_send - m_write_idx;
            }
            if(temp <= -1){
                if(errno == EAGAIN){
                    write_ok = true;
                    if (g_reactor->RegisterHandler(this, reactor::kWriteEvent) != 0){
                        printf("error: register handler failed\n");
                        //delete handler;
                    }
                    return;
                }
                write_ok = false;
                return;
            }
            //bytes_have_send += temp;
            if(bytes_have_send >= m_iv[0].iov_len){
                //继续发送，然后把前面发送的m_iv清零
                m_iv[0].iov_len = 0;
                //m_write_idx是第一段的报文结尾的地方
                m_iv[1].iov_base = m_file_address + newadd;
                m_iv[1].iov_len = bytes_to_send;
            }
            else{
                //指针移动，长度改变
                m_iv[0].iov_base = m_write_buf + bytes_to_send;
                m_iv[0].iov_len = m_iv[0].iov_len - bytes_have_send;
            }
            //数据发送完成
            if(bytes_to_send <= 0){
                unmap();
                if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0){
                    printf("error: register handler failed\n");
                    //delete handler;
                }
                if(m_linger){
                    init();
                    write_ok = true;
                    return;
                }
                else{
                    write_ok = false;
                    return;
                }
            }
        }
    }
    void HandleError() override{
        g_reactor->RemoveHandler(this);
    }
    reactor::handler_t GetHandle() override{
        return m_handle;
    }
    void setHandle(reactor::handler_t thandle) override{
        m_handle = thandle;
    }
    void process();
private:
    void init(char *root,int close_log, std::string user, std::string passwd, std::string sqlname);
    void init();
    HTTPCODE process_read();
    bool process_write(HTTPCODE ret);
    HTTPCODE parse_request_line(std::string text);
    HTTPCODE parse_headers(std::string text);
    HTTPCODE parse_content(std::string text);
    HTTPCODE do_request();
    void umap();
    bool add_response(const char*format, ...);
    bool add_content(const char*content);
    bool add_status_line(int status, const char* title);
    bool add_headers(int content_length);
    bool add_content_type();
    bool add_content_length(int content_length);
    bool add_linger();
    bool add_blank_line();
    void unmap();
    inline bool is_ok(std::string& text);
public:
    static int m_user_count;
    //MYSQL *mysql;
    int m_state;
    static const int FILENAME_LEN = 200;
    static const int READ_BUFFER_SIZE = 2048;
    static const int WRITE_BUFFER_SIZE = 1024;
    std::string userkey = "user";
    std::string passwordkey = "password";
private:
    bool readok;
    bool writeok;
    //EPOLL相关参数
    //handle  读缓冲区 写缓冲区
    reactor::handler_t m_handle; 
    char m_read_buf[READ_BUFFER_SIZE];
    char m_write_buf[WRITE_BUFFER_SIZE];
    char m_real_file[FILENAME_LEN];
    int bytes_to_send;
    int bytes_have_send;
    
    //RequstHeader、ResponseHeader
    bool m_linger;  //是否是长连接
    int m_content_length;   //消息体大小(Request or Response)
    char m_url[20];            //url
    char m_version[20];        //HTTP的版本号
    char m_host[20];           //主机号
    int cgi;            //是否是POST方法
    METHOD m_method;        //method
    std::unordered_map<std::string, std::string> header; //请求头的数据
    std::unordered_map<std::string, std::string> datamap;   //请求体的数据
    
    //状态机
    CHECK_STATE m_check_state;

    //文件相关
    struct stat m_file_stat;  //文件相关
    struct iovec m_iv[2];
    int m_iv_count;
    char* doc_root;     //根目录解析
    char* m_file_address;   //文件地址

    //数据库缓冲
    std::unordered_map<std::string,std::string> m_users;


    //配置相关
    //LT or ET  and 日志
    int m_TRIGMode;
    int m_close_log;
    char m_string[20]; //存储请求头数据
    char sql_user[100];
    char sql_passwd[100];
    char sql_name[100];

    int m_read_idx;
    int m_write_idx;
    int m_checked_idx;
    int m_start_line;

    bool write_ok;
    bool read_ok;
    std::mutex m_lock;
    std::shared_ptr<MYSQLRAII> tmp;
    RedisUtils* redisUtils;
};
#endif