#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include "reactor.h"
#include "singleton.h"
#include "test_common.h"
#include "timeheap.h"
#include <unistd.h>
#include "Config.h"
#include "HttpHandler.h"
typedef Singleton<Connection_Pool> SPool;
#define connPool SPool::getInstance()
typedef Singleton<Config> SConfig;
#define config SConfig::getInstance()
const size_t kBufferSize = 1024;
char g_read_buffer[kBufferSize];
char g_write_buffer[kBufferSize];
typedef Singleton<reactor::Reactor> Sreactor;
#define g_reactor Sreactor::getInstance()
//RequestHandler
class RequestHandler : public reactor::EventHandler{
public:
    reactor::handler_t GetHandle() override{
        return m_handle;
    }
    void HandleRead() override{
        memset(g_read_buffer, 0, sizeof(g_read_buffer));
        int len = recv(m_handle, g_read_buffer, kBufferSize, 0);
        if (len > 0)
        {
            printf("This is a message : %s\n",g_read_buffer);
            if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0)
            {
                printf("error: register handler failed\n");
                //delete handler;
            }
            /*if (strncasecmp("time", g_read_buffer, 4) == 0)
            {
                g_reactor->RegisterHandler(this, reactor::kWriteEvent);
            }
            else if (strncasecmp("exit", g_read_buffer, 4) == 0)
            {
                close(m_handle);
                g_reactor->RemoveHandler(this);
                delete this;
            }
            else
            {
                fprintf(stderr, "Invalid request: %s", g_read_buffer);
                close(m_handle);
                g_reactor->RemoveHandler(this);
                delete this;
            }*/
        }
        else
        {
            ReportSocketError("recv");
        }
    }
    void HandleWrite() override{
        struct tm *ttime;
        char now[64];
        time_t tt;

        memset(now, 0, 64);
        tt = time(NULL);
        ttime = localtime(&tt);
        strftime(now, 64, "%Y-%m-%d %H:%M:%S", ttime);        

        memset(g_write_buffer, 0, sizeof(g_write_buffer));
        int len = sprintf(g_write_buffer, "current time: %s\r\n", now);
        len = send(m_handle, g_write_buffer, len, 0);
        if (len > 0)
        {
            fprintf(stderr, "send response to client, fd=%d\n", (int)m_handle);
            //g_reactor->RegisterHandler(this, reactor::kReadEvent);
            if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0)
            {
                printf("error: register handler failed\n");
                //delete handler;
            }
        }
        else
        {
            ReportSocketError("send");
        }
    }
    void HandleError() override{
        fprintf(stderr, "client %d closed\n", m_handle);
        //close(m_handle);
        g_reactor->RemoveHandler(this);
        delete this;
    }
    void setHandle(reactor::handler_t thandle) override{
        m_handle = thandle;
    }
    RequestHandler(){}
    RequestHandler(reactor::handler_t handle) :
        EventHandler(),
        m_handle(handle)
    {}
    ~RequestHandler(){}
private:
    reactor::handler_t m_handle;
};
RegisterProduct(reactor::EventHandler, RequestHandler)

//建立一个Accept的handler
class AcceptHandler : public reactor::EventHandler{
public:
    AcceptHandler(){}
    AcceptHandler(reactor::handler_t handle) :
        EventHandler(),
        m_handle(handle)
    {}
    reactor::handler_t GetHandle() override{
        return m_handle;
    }
    void HandleRead() override{
        struct sockaddr addr;
        socklen_t addrlen = sizeof(addr);
        reactor::handler_t handle = accept(m_handle, &addr, &addrlen);
        if (!IsValidHandle(handle))
        {
            ReportSocketError("accept");
        }
        else
        {
            reactor::EventHandler * handler = AbstractProductTool::ProductFactory<reactor::EventHandler>::instance().createProduct("HTTPHandler");
            handler->setHandle(handle);
            std::cout<<"successful connect = "<<handle<<std::endl;
            if (g_reactor->RegisterHandler(handler, reactor::kReadEvent) != 0)
            {
                printf("error: register handler failed\n");
                //delete handler;
            }
        }
    }
    void setHandle(reactor::handler_t thandle) override{
        m_handle = thandle;
    }
    void HandleWrite() override{
    }
    void HandleError() override{
    }
    ~AcceptHandler(){}
private:
    reactor::handler_t m_handle;
};
RegisterProduct(reactor::EventHandler, AcceptHandler)
RegisterProduct(reactor::EventHandler, HTTPHandler)
void printHelloworld(client_data* data){
    fprintf(stderr, "timertask : Hello world from timerTask!\n");
}
void Init(){
    //数据库连接初始化
    connPool->setMaxConn(config->sql_num);
    connPool->Init();
    //RPC初始化
    std::string config_file;
    config_file = "rpc.conf";
    RpcApplication::init(config_file);
}
int main(int argc, char *argv[]){
    std::cout<<"yes"<<std::endl;
    config->parse_arg(argc, argv);
    char *m_ip = new char[30]; 
    strcpy(m_ip, config->IP.c_str());
    int m_port = config->PORT;
    Init();
    reactor::handler_t m_handle = socket(AF_INET, SOCK_STREAM, 0);
    if(!IsValidHandle(m_handle)){
        ReportSocketError("create");
        return EXIT_FAILURE;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = inet_addr(m_ip);
    if (bind(m_handle, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        ReportSocketError("bind");
        //printf("bind is error!\n");
        return EXIT_FAILURE;
    }
    if (listen(m_handle, 10) < 0){
        ReportSocketError("listen");
        return EXIT_FAILURE;
    }
    std::cout<<"all prepare work is ok!"<<std::endl;
    //创建监听的模板类
    reactor::EventHandler* server = AbstractProductTool::ProductFactory<reactor::EventHandler>::instance().createProduct("AcceptHandler");
    server->setHandle(m_handle);
    std::cout<<"Server: m_handle = "<<m_handle<<std::endl;
    g_reactor->RegisterHandler(server, reactor::kReadEvent, 1);
    std::cout<<"regsiter the handler"<<std::endl;
    //heap_timer* printtask = new heap_timer(5);
    //printtask->cb_func = printHelloworld;
    //g_reactor->RegisterTimerTask(printtask);
    std::cout<<"regsiter the handler end"<<std::endl;
    while(true){
        //std::cout<<"Work()"<<std::endl;
        g_reactor->Work();
        //sleep(1);
    }
    //Init 线程池？
    //Init GReactor
    //注册监听事件
    //在固定事件开启服务器
    return 0;
}