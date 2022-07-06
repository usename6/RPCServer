#ifndef _DBTEST_
#define _DBTEST_
#include <bits/stdc++.h>
#include <memory>
#include <functional>
#include <list>
#include <mysql/mysql.h>
#include "Config.h"
#include "singleton.h"
typedef Singleton<Config> SConfig;
#define config SConfig::getInstance()
class MYSQLRAII{
public:
    MYSQLRAII(){
        mysql = new MYSQL();
        CloseTag = true; 
    }
    ~MYSQLRAII(){
    }
    void init(std::string url, std::string User, std::string PassWord, std::string DBName, int Port){
        mysql = mysql_init(mysql);
        if(mysql == NULL){
            std::cout<<"init failed"<<std::endl;
            return;
        }
        mysql = mysql_real_connect(mysql, url.c_str(), User.c_str(), PassWord.c_str(), DBName.c_str(), Port, NULL, 0);
        if(mysql == NULL){
            std::cout<<"connect failed"<<std::endl;
            return;
        }
        //test(conn);
        std::cout<<"successful create"<<std::endl;
    }
    void close(){
        if(mysql != NULL)mysql_close(mysql);
    }
    void setClose(){
        CloseTag = true;
    }
    void setUnclose(){
        CloseTag = false;
    }
    bool getCloseTag(){
        return CloseTag;
    }
    MYSQL* getMYSQL(){
        return mysql;
    }
private:
    MYSQL* mysql;
    bool CloseTag;
};
class Connection_Pool{
public:
    Connection_Pool();
    Connection_Pool(int maxconn);
    Connection_Pool(int maxconn, std::string path);
    Connection_Pool(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn, int close_log);
    void init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn, int close_log); 
    std::shared_ptr<MYSQLRAII> getNewConn();
    int getMaxConn();
    void setMaxConn(int MaxConn);
    void Init();
    //int Connection_Pool::GetFreeConn();
    void DestroyPool();
    void show();
    ~Connection_Pool();
    //void DeleteFunc(MYSQLRAII* conn);
    void submit(MYSQLRAII* conn);
    std::function<void(MYSQLRAII*)> DeleteFunc;
    //std::function<void(MYSQLRAII*)> submit;
private:
    int maxConn;
    int m_FreeConn;
    std::list<std::shared_ptr<MYSQLRAII>> connList;
    std::mutex m_mutex;
    std::string m_url;
    int m_port;
    std::string m_user;
    std::string m_password;
    std::string m_DataBaseName;
    std::string filename;
    int Close_Log;
};
#endif