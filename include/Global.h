#ifndef Global_H_
#define Global_H_
#include <string>
#include <google/protobuf/service.h>
#include <google/protobuf/descriptor.h>
#include <iostream>
#include <cstdio>
#include <unordered_map>
#include "ZookeeperClient.hpp"
//服务类型信息
struct ServiceInfo{
    //保存服务对象
    google::protobuf::Service *service_;
    //保存服务方法：    <服务名，指向方法指针>
    std::unordered_map<std::string, const google::protobuf::MethodDescriptor *> method_map_;
};
extern std::unordered_map<std::string, ServiceInfo> service_map_;
extern ZookeeperClient zk_client;
void notify_service(google::protobuf::Service *service);
void run(std::string ip, int port);
#endif 