#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <time.h>
#include "reactor.h"
#include "singleton.h"
#include "test_common.h"
#include <unistd.h>
const size_t kBufferSize = 1024;
char g_read_buffer[kBufferSize];
char g_write_buffer[kBufferSize];
typedef Singleton<reactor::Reactor> Sreactor;
#define g_reactor Sreactor::getInstance()
class RequestHandler : public reactor::EventHandler{
public:
    reactor::handler_t GetHandle() override{
        return m_handle;
    }
    void HandleRead() override{
        memset(g_read_buffer, 0, kBufferSize);
        int len = recv(m_handle, g_read_buffer, kBufferSize, 0);
        if (len > 0)
        {
            fprintf(stderr, "%s", g_read_buffer);
            g_reactor->RegisterHandler(this, reactor::kReadEvent);
        }
        else
        {
            ReportSocketError("recv");
        }
    }

    void HandleWrite() override{
        memset(g_write_buffer, 0, kBufferSize);
        int len = sprintf(g_write_buffer, "Hello Server, I am usename6\r\n");
        len = send(m_handle, g_write_buffer, len, 0);
        if (len > 0){
            fprintf(stderr, "%s", g_write_buffer);
            //g_reactor->RegisterHandler(this, reactor::kReadEvent);
        }
        else{
            ReportSocketError("send");
        }
    }

    virtual void HandleError() override{
        fprintf(stderr, "server closed\n");
        //close(m_handle);
        exit(0);
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
int main(int argc, char ** argv){
    if(argc < 3){
        return EXIT_FAILURE;
    }
    const char *m_ip = argv[1];
    unsigned short m_port = atoi(argv[2]);
    reactor::handler_t m_handle = socket(AF_INET, SOCK_STREAM, 0);
    if(!IsValidHandle(m_handle)){
        ReportSocketError("create");
        return EXIT_FAILURE;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(m_port);
    addr.sin_addr.s_addr = inet_addr(m_ip);
    std::cout<<"Ip address = "<< inet_addr(m_ip)<<" " <<m_ip<<std::endl;
    std::cout<<"m_port = " << htons(m_port) << " " << m_port << std::endl;
    if (connect(m_handle, (struct sockaddr *)&addr, sizeof(addr)) < 0){
        ReportSocketError("connect");
        return EXIT_FAILURE;
    }
    reactor::EventHandler* client = AbstractProductTool::ProductFactory<reactor::EventHandler>::instance().createProduct("RequestHandler");
    client->setHandle(m_handle);
    g_reactor->RegisterHandler(client, reactor::kWriteEvent);
    while(true){
        sleep(4);
        //std::cout<<"Client RegisterHandler"<<std::endl;
        g_reactor->RegisterHandler(client, reactor::kWriteEvent);
        g_reactor->Work();
        
    }
}