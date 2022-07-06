#include "Global.h"
#include "RpcLogger.hpp"
#include "ZookeeperClient.hpp"
//框架提供给外部使用的，可以发布rpc方法的函数接口
ZookeeperClient zk_client;
void notify_service(google::protobuf::Service *service)
{
    ServiceInfo service_info;
    //获取服务对象的描述信息
    const google::protobuf::ServiceDescriptor *service_desc = service->GetDescriptor();

    //获取服务对象名字
    std::string service_name = service_desc->name();
    //获取服务对象方法的数量
    int method_nums = service_desc->method_count();
    printf("service_name = %s\n",service_name.c_str());
    printf("method_nums = %d\n",method_nums);

    //cout << "service name: " << service_name << endl;
    for (int i = 0; i < method_nums; ++i)
    {
        //获取服务对象指定下标的服务的描述
        const google::protobuf::MethodDescriptor *method_desc = service_desc->method(i);
        std::string method_name = method_desc->name();
        printf("method_name = %s\n",method_name.c_str());
        //cout << "method name: " << method_name << endl;
        //插入信息
        service_info.method_map_.insert({method_name, method_desc});
    }
    service_info.service_ = service;
    service_map_.insert({service_name, service_info});
}

//启动rpc服务节点，开始提供rpc远程网络调用服务
void run(std::string ip, int port)
{
    //把当前rpc节点上要发布的服务全部注册到zk上面，让rpc client可以从zk上发现服务
    zk_client.start();
    //在配置中心中创建节点
    for (auto &sp : service_map_)
    {
        std::string service_path = "/" + sp.first;
        zk_client.create(service_path.c_str(), nullptr, 0);
        for (auto &mp : sp.second.method_map_)
        {
            std::string method_path = service_path + "/" + mp.first;
            char method_path_data[128] = {0};
            sprintf(method_path_data, "%s:%d", ip.c_str(), port);
            printf("method_path = %s\n",method_path.c_str());
            printf("method_path_data = %s\n",method_path_data);
            //ZOO_EPHEMERAL 表示znode时候临时性节点
            zk_client.create(method_path.c_str(), method_path_data, strlen(method_path_data), ZOO_EPHEMERAL);
        }
    }

    printf("[info]:server RpcProvider [ip: %s][port: %d]\n", ip.c_str(), port);
    //启动网络服务
}
std::unordered_map<std::string, ServiceInfo> service_map_;