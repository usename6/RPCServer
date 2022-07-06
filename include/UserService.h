#ifndef USERSERVICE_H_
#define USERSERVICE_H_
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <string>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <iostream>
#include "User.pb.h"
#include "RpcApplication.hpp"
#include "RpcProvider.hpp"
#include "singleton.h"
#include "test_common.h"
#include "timeheap.h"
#include <unistd.h>
#include <memory>
#include <mutex>
#include "Config.h"
#include "DataBase_Pool.h"
#include "RedisUtils.h"
typedef Singleton<Connection_Pool> SPool;
#define connPool SPool::getInstance()
typedef Singleton<Config> SConfig;
#define config SConfig::getInstance()
class UserService : public ik::UserServiceRpc //使用在rpc服务发布端
{
public:
    UserService(){
        tmp = connPool->getNewConn();
        redisUtils = new RedisUtils();
        //在user表中检索username，passwd数据，浏览器端输入
        /*if (mysql_query(mysql, "SELECT username,passwd FROM user")){
            std::cout<<"SELECT error:"<<mysql_error(mysql)<<std::endl;
            //LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
            return;
        }*/
        //改一下不查全部的。。。
        //从表中检索完整的结果集
        /*MYSQL_RES *result = mysql_store_result(mysql);
        //返回结果集中的列数
        int num_fields = mysql_num_fields(result);
        //返回所有字段结构的数组
        MYSQL_FIELD *fields = mysql_fetch_fields(result);
        //从结果集中获取下一行，将对应的用户名和密码，存入map中
        while (MYSQL_ROW row = mysql_fetch_row(result)){
            std::string temp1(row[0]);
            std::string temp2(row[1]);
            m_users[temp1] = temp2;
        }*/
    }
    bool Login(string name, string password)
    {
        //step 1.redis查询
        DAO::pUser* user = new DAO::pUser();
        user->set_username(name);
        user->set_password(password);
        DAO::pUser* quser = new DAO::pUser();
        redisUtils->HGETALL("service0",name,*user);
        if(quser->username()==name&&quser->password()==password){
            return true;
        }
        //step 2.login调用select函数
        //step 3.查询失败加入redis中
        MYSQL *mysql = tmp->getMYSQL();
        std::cout<<"username = "<<name<<std::endl;
        std::cout<<"password = "<<password<<std::endl;
        std::string command = "SELECT username,passwd FROM user where username = \""  + name + "\"";
        if (mysql_query(mysql, command.c_str())){
            std::cout<<"SELECT error:"<<mysql_error(mysql)<<std::endl;
            //LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
            return false;
        }
        MYSQL_RES *result = mysql_store_result(mysql);
        MYSQL_FIELD *fields = mysql_fetch_fields(result);
        MYSQL_ROW row = mysql_fetch_row(result);
        std::string temp1(row[0]);
        std::string temp2(row[1]);
        std::cout<<"find username = "<<temp1<<std::endl;
        std::cout<<"find password = "<<temp2<<std::endl;
        if(temp1 == name && temp2 == password){
            redisUtils->HSMSET("service0",name,user);
            return true;
        }
        return false;
    }

    bool Register(uint32_t id, string name, string password)
    {
        cout << "doing local service:register: ";
        cout << "id:" << id << "name: " << name << " password: " << password << endl;
        if(m_users.find(name) == m_users.end()){
            char *sql_insert = (char *)malloc(sizeof(char) * 200);
            strcpy(sql_insert, "INSERT INTO user(username, passwd) VALUES(");
            strcat(sql_insert, "'");
            strcat(sql_insert, name.c_str());
            strcat(sql_insert, "', '");
            strcat(sql_insert, password.c_str());
            strcat(sql_insert, "')");
            m_lock.lock();
            MYSQL *mysql = tmp->getMYSQL();
            int res = mysql_query(mysql, sql_insert);
            m_lock.unlock();
            std::cout<<"Register result:"<<res<<std::endl;
            if(res){
                //step 1.redis查询
                DAO::pUser* user = new DAO::pUser();
                user->set_username(name);
                user->set_password(password);
                redisUtils->HSMSET("service0",name,user);
            }
            return res;
        }
        return true;
    }

    //重写基类UserServiceRpc的虚函数，供框架调用
    void Login(::google::protobuf::RpcController *controller,
               const ::ik::LoginRequest *request,
               ::ik::LoginResponse *response,
               ::google::protobuf::Closure *done)
    {
        //框架给业务上报了请求参数 request，业务获取相应数据做本地业务
        string name = request->name();
        string password = request->password();

        //本地业务
        bool login_result = Login(name, password);

        //把响应给调用方返回
        ik::ErrorMsg *errmsg = response->mutable_errmsg();
        errmsg->set_error(0);
        errmsg->set_error_msg("");
        response->set_success(login_result);

        //执行回调操作
        done->Run();
    }

    void Register(::google::protobuf::RpcController *controller,
                  const ::ik::RegisterRequest *request,
                  ::ik::RegisterResponse *response,
                  ::google::protobuf::Closure *done)
    {
        //框架给业务上报了请求参数 request，业务获取相应数据做本地业务
        uint32_t id = request->id();
        string name = request->name();
        string password = request->password();

        //本地业务
        bool login_result = Register(id, name, password);

        //把响应给调用方返回
        ik::ErrorMsg *errmsg = response->mutable_error();
        errmsg->set_error(0);
        errmsg->set_error_msg("");
        response->set_success(login_result);

        //执行回调操作
        done->Run();
    }
private:
    std::mutex m_lock;
    std::shared_ptr<MYSQLRAII> tmp;
    std::unordered_map<std::string,std::string> m_users;
    RedisUtils* redisUtils;
};
#endif