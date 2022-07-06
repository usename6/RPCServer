#include "DataBase_Pool.h"
void Connection_Pool::submit(MYSQLRAII *conn){
    std::shared_ptr<MYSQLRAII> ptr2(conn, DeleteFunc);
    ptr2->setUnclose();
    connList.push_back(std::move(ptr2));
}
/*void Connection_Pool::DeleteFunc(MYSQLRAII *conn){
    std::cout<<"close"<<std::endl;
    int tag=conn->getCloseTag();
    switch(tag){
    case 0:{
        if(conn != NULL)conn->close();
        delete conn;
        break;
    }
    case 1:{
        //submit(conn);
    }
    default:
        break;
    }
}*/
void test(MYSQLRAII& conn){
    if (mysql_query(conn.getMYSQL(), "SELECT username,passwd FROM user")){
        //LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
        std::cout<<"SELECT error"<<std::endl;
    }
    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(conn.getMYSQL());
    //返回结果集中的列数
    int num_fields = mysql_num_fields(result);
    //返回所有字段结构的数组
    MYSQL_FIELD *fields = mysql_fetch_fields(result);

    //从结果集中获取下一行，将对应的用户名和密码，存入map中
    while (MYSQL_ROW row = mysql_fetch_row(result)){
        std::string temp1(row[0]);
        std::string temp2(row[1]);
        std::cout<<"temp1 = "<<temp1<<std::endl;
        std::cout<<"temp2 = "<<temp2<<std::endl;
    }
}
Connection_Pool::Connection_Pool(){}
Connection_Pool::Connection_Pool(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn, int close_log)
    :m_url(url), m_user(User), m_password(PassWord), m_DataBaseName(DataBaseName), m_port(Port), maxConn(MaxConn), Close_Log(close_log){
    init(m_url, m_user, m_password, m_DataBaseName, m_port, MaxConn, Close_Log);
}
Connection_Pool::Connection_Pool(int maxconn):maxConn(maxconn){
    
}

std::shared_ptr<MYSQLRAII> Connection_Pool::getNewConn(){
    m_mutex.lock();
    std::cout<<"move start"<<std::endl;
    std::shared_ptr<MYSQLRAII> ptr3 = connList.front();
    connList.pop_front();
    std::cout<<"move end"<<std::endl;
    m_mutex.unlock();
    test(*std::move(ptr3.get()));
    ptr3->setClose();
    return ptr3;
}
void Connection_Pool::show(){
    std::cout<<"--------------------"<<std::endl;
    std::cout<<"current DataBase:"<<std::endl;
    for(auto conn : connList){
        std::cout<<conn<<std::endl;
    }
    std::cout<<"--------------------"<<std::endl;
}

Connection_Pool::Connection_Pool(int maxconn, std::string path){
    maxConn = maxconn;
}
void Connection_Pool::init(std::string url, std::string User, std::string PassWord, std::string DataBaseName, int Port, int MaxConn, int close_log){
    DeleteFunc = [&](MYSQLRAII *conn){
        int tag=conn->getCloseTag();
        switch(tag){
        case 0:{
            if(conn != NULL)conn->close();
            delete conn;
            break;
        }
        case 1:{
            submit(conn);
        }
        default:
            break;
        }
    };
    std::cout<<"init start"<<std::endl;
    for(int i=0;i<maxConn;i++){
        std::shared_ptr<MYSQLRAII> ptr(new MYSQLRAII(),DeleteFunc);
        ptr->init(url, User, PassWord, DataBaseName, Port);
        //test(*(std::move(ptr.get())));
        connList.emplace_back(std::move(ptr));
        std::cout<<connList.back()<<std::endl;
    }
}
void Connection_Pool::setMaxConn(int MaxConn){
    maxConn = MaxConn;
}
void Connection_Pool::Init(){
    m_url = config->DBIP;
    m_user = config->username;
    m_password = config->password;
    m_DataBaseName = config->DBName;
    m_port = config->DBPORT;
    maxConn = maxConn;
    Close_Log = config->close_log;
    std::cout<<"Connection Pool Init start!"<<std::endl;
    init(m_url, m_user, m_password, m_DataBaseName, m_port, maxConn, Close_Log);
}
void Connection_Pool::DestroyPool(){
    m_mutex.lock();
	if (connList.size() > 0){
		connList.clear();
	}
	m_mutex.unlock();
}
Connection_Pool::~Connection_Pool(){
    this->DestroyPool();
}