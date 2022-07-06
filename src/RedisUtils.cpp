#include "RedisUtils.h"
const char* TypeToName(const char* name)
{
    const char* __name = abi::__cxa_demangle(name, nullptr, nullptr, nullptr);
    return __name;
}
RedisUtils::RedisUtils(){
    start();
}
RedisUtils::~RedisUtils(){

}
//启动redis
void RedisUtils::start(){
    //redis默认监听端口为6387 可以再配置文件中修改 
    c = redisConnect("127.0.0.1", 6379); 
}