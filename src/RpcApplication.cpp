#include "RpcApplication.hpp"

#include <iostream>
#include <unistd.h>
#include <string>

using namespace std;

RpcConfigure RpcApplication::configure_;

void RpcApplication::init(int argc, char **argv)
{
    if (argc < 2)
    {
        std::cout << "format: command -i <configfile>"<<std::endl;
    }

    int choose = 0;
    std::string config_file;
    while ((choose = getopt(argc, argv, "i:")) != -1)
    {
        switch (choose)
        {
        case 'i':
            config_file = optarg;
            break;
        case '?':
            std::cout << " format: command -i <configfile>" << std::endl;
            break;
        case ':':
            std::cout << " format: command -i <configfile>" << std::endl;
            break;
        default:
            break;
        }
    }
    //加载配置文件
    configure_.load_configure(config_file.c_str());
}
void RpcApplication::init(std::string config_file){
    //加载配置文件
    configure_.load_configure(config_file.c_str());
}
RpcApplication::RpcApplication()
{
}
