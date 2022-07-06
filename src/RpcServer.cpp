#include "RpcServerHandler.h"
#include "UserService.h"
#include "RpcAcceptHandler.h"
#include "reactor.h"
typedef Singleton<Connection_Pool> SPool;
#define connPool SPool::getInstance()
typedef Singleton<Config> SConfig;
#define config SConfig::getInstance()
typedef Singleton<reactor::Reactor> Sreactor;
#define g_reactor Sreactor::getInstance()
RegisterProduct(reactor::EventHandler, RpcAcceptHandler)
RegisterProduct(reactor::EventHandler, RpcServerHandler)
void Init(){
    connPool->setMaxConn(config->sql_num);
    connPool->Init();
}
int main(int argc, char **argv){
    //RPC_LOG_INFO("HELLO");
    //RPC_LOG_ERROR("%s,%s,%d", __FILE__, __FUNCTION__, __LINE__);
    //调用框架的初始化操作
    RpcApplication::init(argc, argv);
    std::string ip = RpcApplication::get_instance().get_configure().find_load("rpcserver_ip");
    uint16_t port = (uint16_t)atoi(RpcApplication::get_instance().get_configure().find_load("rpcserver_port").c_str());
    char *m_ip = new char[30]; 
    strcpy(m_ip, ip.c_str());
    int m_port = port;
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
    reactor::EventHandler* server = AbstractProductTool::ProductFactory<reactor::EventHandler>::instance().createProduct("RpcAcceptHandler");
    server->setHandle(m_handle);
    g_reactor->RegisterHandler(server, reactor::kReadEvent, 1);
    notify_service(new UserService());
    run(ip, port);
    std::cout<<"register success!"<<std::endl;
    while(true){
        g_reactor->Work();
    }
    return 0;
}