#ifndef _RPCSERVERHANDLER_H_
#define _RPCSERVERHANDLER_H_
#include "EventHandler.h"
#include "Global.h"
#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <iostream>
#include <cstdio>
#include <unordered_map>
#include <string>
#include <iostream>
#include <functional>
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
#include "reactor.h"
#include "singleton.h"
#include "test_common.h"
#include "timeheap.h"
#include "RpcApplication.hpp"
#include "RpcHeader.pb.h"
#include "RpcLogger.hpp"
#include "Config.h"
#include "ZookeeperClient.hpp"
const int kBufferSize = 1024;
typedef Singleton<reactor::Reactor> Sreactor;
#define g_reactor Sreactor::getInstance()
class RpcServerHandler : public reactor::EventHandler{
public:
    RpcServerHandler(){}
    RpcServerHandler(reactor::handler_t handle):
        m_handle(handle)
    {}
    reactor::handler_t GetHandle() override{
        return m_handle;
    }
    void HandleRead() override{
        memset(read_buffer, 0, sizeof(read_buffer));
        int len = recv(m_handle, read_buffer, kBufferSize, 0);
        std::string recv_buf;
        for(int i=0;i<len;i++)recv_buf.push_back(read_buffer[i]);
        //从字符流中读取前四个字节的内容(利用int特性),即header的长度
        uint32_t header_size = 0;
        recv_buf.copy((char *)&header_size, 4, 0);
        //根据header_size读取数据头的原始字符流
        std::string rpc_header_str = recv_buf.substr(4, header_size);
        //反序列化
        ikrpc::RpcHeader rpc_header;
        std::string service_name;
        std::string method_name;
        uint32_t args_size = 0;
        if (rpc_header.ParseFromString(rpc_header_str)){
            //数据头反序列化成功
            service_name = rpc_header.service_name();
            method_name = rpc_header.method_name();
            args_size = rpc_header.args_size();
        }
        else{
            //数据头反序列化失败
            printf("[error]:rpc header str:%s parse error!", rpc_header_str.c_str());
            return;
        }

        //解析参数
        std::string args_str = recv_buf.substr(4 + header_size, args_size);

        //打印调试信息
        printf("[info]:<------------------------------------------------>\n");
        printf("[info]:rpc header str: %s\n", rpc_header_str.c_str());
        printf("[info]:service name: %s\n", service_name.c_str());
        printf("[info]:method name: %s\n", method_name.c_str());
        printf("[info]:args: %s\n", args_str.c_str());
        printf("[info]:<------------------------------------------------>\n");

        //获取service对象和method对象
        auto service_it = service_map_.find(service_name);
        if (service_it == service_map_.end()){
            printf("[error]:%s is not exist\n", service_name.c_str());
            return;
        }
        auto method_it = service_it->second.method_map_.find(method_name);
        if (method_it == service_it->second.method_map_.end()){
            printf("[error]:%s::%s is not exist\n", service_name.c_str(), method_name.c_str());
            return;
        }

        //获取service和method对象
        google::protobuf::Service *service = service_it->second.service_;
        const google::protobuf::MethodDescriptor *method = method_it->second;

        //生成rpc方法调用的请求request 和响应response参数
        google::protobuf::Message *request = service->GetRequestPrototype(method).New();
        if (!request->ParseFromString(args_str)){
            printf("[error]:request parse error!\n");
            return;
        }
        response = service->GetResponsePrototype(method).New();

        //给callMethod方法的调用，绑定一个closure回调函数
        google::protobuf::Closure *done = google::protobuf::NewCallback<RpcServerHandler>(this, &RpcServerHandler::HandleWrite);

        //在框架上根据远端rpc请求，调用响应方法
        service->CallMethod(method, nullptr, request, response, done);
    }
    void setHandle(reactor::handler_t thandle) override{
        m_handle = thandle;
    }
    void HandleWrite() override{
        std::string response_str;
        //序列化
        if (response->SerializeToString(&response_str)){
            //发送序列化的数据
            std::cout<<"response_str = "<<response_str<<std::endl;
            std::cout<<"size = "<<response_str.size()<<std::endl;
            uint32_t rvalue = 0;
            response_str.copy((char *)&rvalue, 4, 0);
            std::cout<<"rvalue = "<<(int)rvalue<<std::endl;
            int len = send(m_handle, response_str.c_str(), response_str.size(), 0);
            std::cout<<"len = "<<len<<std::endl;
            if (len > 0){
                fprintf(stderr, "send response to client, fd=%d\n", (int)m_handle);
                //g_reactor->RegisterHandler(this, reactor::kReadEvent);
                if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0){
                    printf("[error]:register handler failed\n");
                    //delete handler;
                }
            }
            else{
                printf("[error]:send error\n");
            }
        }
        else{
            //序列化失败
            printf("[error]:serialize reponse error");
        }
    }
    void HandleError() override{
    }
    ~RpcServerHandler(){}
    
private:
    reactor::handler_t m_handle;
    //存储服务信息
    google::protobuf::Message *response;
    std::string response_str;
    char read_buffer[1024];
    char write_buffer[1024];
};
#endif