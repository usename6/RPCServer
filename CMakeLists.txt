cmake_minimum_required(VERSION 3.5)
project(HttpServer)          #定义工程的名字
# 设置指定的C++编译器版本是必须的，如果不设置，或者为OFF，则指定版本不可用时，会使用上一版本。
set(CMAKE_CXX_STANDARD_REQUIRED ON)
# 指定为C++11 版本
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -pthread")
find_package(Threads REQUIRED)

SET(CMAKE_BUILD_TYPE DEBUG)  #定义编译的TYPE 这里我选的是DEBUG
set(EXECUTABLE_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/bin)   #可执行文件保存的位置
set(LIBRARY_OUTPUT_PATH ${PROJECT_SOURCE_DIR}/lib)      #输出的链接库保存的位置
include_directories(
    ${PROJECT_SOURCE_DIR}/include                       #静态库目录
)

add_library(HTTPLIB SHARED src/event_demultiplexer.cpp 
                           src/EventHandler.cpp
                           src/ThreadPool.cpp
                           src/reactor.cpp
                           src/DataBase_Pool.cpp
                           src/HttpHandler.cpp
                           src/Log.cpp
                           src/Config.cpp
                           src/RpcApplication.cpp
                           src/RpcChannel.cpp
                           src/RpcConfigure.cpp
                           src/RpcControl.cpp
                           src/RpcHeader.pb.cc
                           src/RpcLogger.cpp
                           src/User.pb.cc
                           src/RpcLogger.cpp
                           src/Global.cpp
                           src/ZookeeperClient.cpp
                           src/RedisUtils.cpp
                           src/pUser.pb.cc)
link_directories(/usr/lib/mysql)
link_directories(/usr/local/lib)
target_link_libraries(HTTPLIB hiredis)
target_link_libraries(HTTPLIB libmysqlclient.so)
target_link_libraries(HTTPLIB protobuf)
target_link_libraries(HTTPLIB zookeeper_mt)
target_link_libraries(HTTPLIB Threads::Threads)
#target_link_libraries(HTTPLIB muduo_base)
#target_link_libraries(HTTPLIB libmysqlclient.so)
AUX_SOURCE_DIRECTORY(src DIR_SRCS)                      #找到src目录对应的值
add_executable(HttpServer src/Server.cpp)                  #单个main函数的编译结果
target_link_libraries(HttpServer Threads::Threads)
#target_link_libraries(HttpServer libmysqlclient.so protobuf zookeeper_mt)
target_link_libraries(HttpServer HTTPLIB)

add_executable(HttpClient src/Client.cpp)   
target_link_libraries(HttpClient HTTPLIB)

add_executable(HttpRpcServer src/RpcServer.cpp)
target_link_libraries(HttpRpcServer HTTPLIB)