#ifndef RPCACCEPTHANDLER_H_
#define RPCACCEPTRPCHANDLER_H_
#include "EventHandler.h"
#include "Global.h"
#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <sys/types.h>
#include <sys/socket.h>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
//RegisterProduct(reactor::EventHandler, RpcServerHandler)
//建立一个Accept的handler
class RpcAcceptHandler : public reactor::EventHandler{
public:
    RpcAcceptHandler(){}
    RpcAcceptHandler(reactor::handler_t handle) :
        EventHandler(),
        m_handle(handle)
    {}

    reactor::handler_t GetHandle() override{
        return m_handle;
    }
    void HandleRead() override{
        struct sockaddr addr;
        socklen_t addrlen = sizeof(addr);
        std::cout<<"Handle Read ready!"<<std::endl;
        reactor::handler_t handle = accept(m_handle, &addr, &addrlen);
        if (!IsValidHandle(handle))
        {
            printf("[error]:accept error!\n");
        }
        else
        {
            reactor::EventHandler * handler = AbstractProductTool::ProductFactory<reactor::EventHandler>::instance().createProduct("RpcServerHandler");
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
    ~RpcAcceptHandler(){}
private:
    reactor::handler_t m_handle;
    unordered_map<std::string, ServiceInfo> service_map_;
};
#endif