#include "HttpHandler.h"
#include <mysql/mysql.h>
#include "RedisUtils.h"
#include "pUser.pb.h"
#include "Log.h"
//定义http响应的一些状态信息
const char *ok_200_title = "OK";
const char *error_400_title = "Bad Request";
const char *error_400_form = "Your request has bad syntax or is inherently impossible to staisfy.\n";
const char *error_403_title = "Forbidden";
const char *error_403_form = "You do not have permission to get file form this server.\n";
const char *error_404_title = "Not Found";
const char *error_404_form = "The requested file was not found on this server.\n";
const char *error_500_title = "Internal Error";
const char *error_500_form = "There was an unusual problem serving the request file.\n";
void HTTPHandler::initmysql_result(){
    //先从连接池中取一个连接
    tmp = connPool->getNewConn();
    MYSQL *mysql = tmp->getMYSQL();
    //在user表中检索username，passwd数据，浏览器端输入
    if (mysql_query(mysql, "SELECT username,passwd FROM user")){
        std::cout<<"SELECT error:"<<mysql_error(mysql)<<std::endl;
        LOG_ERROR("SELECT error:%s\n", mysql_error(mysql));
        return;
    }
    //从表中检索完整的结果集
    MYSQL_RES *result = mysql_store_result(mysql);
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
        m_users[temp1] = temp2;
    }
}
void HTTPHandler::unmap(){
    if(m_file_address){
        //函数说明 munmap()用来取消参数start所指的映射内存起始地址，
        //参数length则是欲取消的内存大小。
        //当进程结束或利用exec相关函数来执行其他程序时，
        //映射内存会自动解除，但关闭对应的文件描述符时不会解除映射。
        munmap(m_file_address, m_file_stat.st_size);
        m_file_address = 0;
    }
}
//初始化连接,外部调用初始化套接字地址
void HTTPHandler::init(char *root,int close_log, 
                    std::string user, std::string passwd, std::string sqlname)
{

    //当浏览器出现连接重置时，可能是网站根目录出错或http响应格式出错或者访问的文件中内容完全为空
    doc_root = root;
    m_close_log = close_log;

    strcpy(sql_user, user.c_str());
    strcpy(sql_passwd, passwd.c_str());
    strcpy(sql_name, sqlname.c_str());

    init();
}

void HTTPHandler::init(){
    //mysql = NULL;
    bytes_to_send = 0;
    bytes_have_send = 0;
    m_check_state = CHECK_STATE_REQUESTLINE;
    m_linger = false;
    m_method = GET;
    //m_url = new char[20];
    memset(m_url,0,sizeof(m_url));
    //m_version = new char[20];
    memset(m_version,0,sizeof(m_version));
    m_content_length = 0;
    //m_host = new char[20];
    memset(m_host,0,sizeof(m_host));
    //m_string = new char[20];
    memset(m_string,0,sizeof(m_string));
    m_start_line = 0;
    m_checked_idx = 0;
    m_read_idx = 0;
    m_write_idx = 0;
    cgi = 0;
    m_state = 0;

    memset(m_read_buf, 0 , READ_BUFFER_SIZE*sizeof(char));
    memset(m_write_buf, 0 , WRITE_BUFFER_SIZE*sizeof(char));
    memset(m_real_file, 0 , FILENAME_LEN*sizeof(char));
}
HTTPHandler::HTTPCODE HTTPHandler::process_read(){
    std::cout<<"process read is ok!"<<std::endl;
    HTTPCODE ret = NO_REQUEST; 
    std::string text;
    while(is_ok(text)){
        switch(m_check_state){
        case CHECK_STATE_REQUESTLINE:{
            //std::cout<<"parse request line in!"<<std::endl;
            ret = parse_request_line(text);
            if(ret == BAD_REQUEST)
                return BAD_REQUEST;
            break;
        }
        case CHECK_STATE_HEADER:{
            std::cout<<"parse header line in!"<<std::endl;
            ret = parse_headers(text);
            if(ret == BAD_REQUEST)
                return BAD_REQUEST;
            else if(ret == GET_REQUEST){
                return do_request();
            }
            break;
        }
        case CHECK_STATE_CONTENT:{
            std::cout<<"parse content line in!"<<std::endl;
            strcpy(m_string,text.c_str());
            ret = parse_content(text);
            if(ret == GET_REQUEST)
                return do_request();
            break;
        }
        default:
            return INTERNAL_ERROR;
        }
    }
    return NO_REQUEST;
}
inline bool HTTPHandler::is_ok(std::string& text){
    std::string str(m_read_buf+m_checked_idx);
    std::cout<<"In:is ok"<<std::endl;
    std::cout<<"m_read_buf = "<<str<<std::endl;
    std::cout<<"m_checked_idx = "<<m_checked_idx<<std::endl;
    switch(m_check_state){
    case CHECK_STATE_REQUESTLINE:{
        //TODO
        //check exist the /r/n
        int p;
        if((p=str.find_first_of("\r\n"))==std::string::npos)return false;
        text = str.substr(0, p);
        //std::cout<<"str["<<(p-1)<<"] = "<<str[p-1]<<std::endl;
        //std::cout<<"Request: text = "<<text<<std::endl;
        m_checked_idx = p+2;
        break;
    }
    case CHECK_STATE_HEADER:{
        //直接判断最后一个字符是不是"\0"
        //TODO 复制str
        //std::cout<<"Header: text = "<<text<<std::endl;
        int p;
        if((p=str.find_last_of("\r\n\r\n"))==std::string::npos){
            return false;
        }
        text = str.substr(0,p-3);
        //std::cout<<"p = "<<p-3<<std::endl;
        //std::cout<<"len = "<<str.size()<<std::endl;
        //std::cout<<"text = "<<text<<std::endl;
        //std::cout<<"pre: m_checked_idx = "<<m_checked_idx<<std::endl;
        m_checked_idx = m_checked_idx + text.size() + 4;
        //std::cout<<"m_checked_idx = "<<m_checked_idx<<std::endl;
        //printf("m_read_buf[%d] = [%c]\n",m_read_idx, m_read_buf[m_read_idx-1]);
        //std::cout<<"m_read_idx = "<<m_read_idx<<std::endl;
        break;
    }
    case CHECK_STATE_CONTENT:{
        if(m_read_idx < (m_content_length + m_checked_idx)){
            return false;
        }
        //TODO
        text = str;
        std::cout<<"content:"<<std::endl;
        std::cout<<"text = "<<text<<std::endl;
        m_checked_idx = m_read_idx;
        break;
    }
    default:
        break;
    }
    std::cout<<"is_ok"<<std::endl;
    return true;
}
HTTPHandler::HTTPCODE HTTPHandler::parse_request_line(std::string text){
    std::stringstream ss(text);
    int cnt=0;
    std::string str,str1;
    while(getline(ss,str,' ')){
        std::stringstream ss1(str);
        while(getline(ss1,str1,'\t')){
            if(cnt==1){
                //std::cout<<"1: str1 = "<<str1<<std::endl;
                if(str1.size()==0)return BAD_REQUEST;
                if(str1.find("http://")!=std::string::npos){
                    str1 = str1.replace(str1.find("http://"), 7, "/");
                    //strcpy(m_url, (str1.replace(str1.find("http://"), 7, "/").append(str1.size()==7?"judge.html":"")).c_str());
                }
                else if(str1.find("https://")!=std::string::npos){
                    //strcpy(m_url, (str1.replace(str1.find("https://"), 8, "/").append(str1.size()==8?"judge.html":"")).c_str());
                    str1 = str1.replace(str1.find("https://"), 8, "/");
                }
                if(str1[0]!='/')return BAD_REQUEST;
                if(str1.size() == 1)str1 += "judge.html";
                strcpy(m_url, str1.c_str());
                //std::cout<<"m_url = "<<m_url<<std::endl;
            }
            else if(cnt==0){
                //std::cout<<"0: str1 = "<<str1<<std::endl;
                if(str1 == "GET")m_method = GET;
                else if(str1 == "POST"){
                    m_method = POST;
                    cgi = 1;
                }
                else return BAD_REQUEST;
            }
            else if(cnt==2){
                if(str1.size()==0)return BAD_REQUEST;
                if(str1 != "HTTP/1.1")return BAD_REQUEST;
                strcpy(m_version, str1.c_str());
            }
            cnt++;
        }
    }
    m_check_state = CHECK_STATE_HEADER;
}
HTTPHandler::HTTPCODE HTTPHandler::parse_headers(std::string text){
    std::stringstream ss(text);
    std::string str;
    std::vector<std::string> lines;
    while(getline(ss,str,'\n')){
        //std::cout<<"str = "<<str<<std::endl;
        std::string str1;
        std::stringstream ss1(str);
        while(getline(ss1,str1,'\r')){
            //std::cout<<"str = "<<str<<std::endl;
            lines.push_back(str1);
        }
    }
    for(auto line:lines){
        //std::cout<<"line:"<<line<<std::endl;
        std::stringstream ss2(line);
        std::string str2;
        std::vector<std::string> d;
        while(getline(ss2,str2,' ')){
            std::stringstream ss3(str2);
            std::string str3;
            while(getline(ss3,str3,'\t')){
                d.push_back(str3);
            }
        }
        for(int i=0;i<d.size();i+=2){
            //std::cout<<"header["<<d[i]<<"] = "<<d[i+1]<<std::endl;
            header[d[i]] = d[i+1];
        }
    }
    for(auto kv:header){
        if(kv.first == "Connection:" && kv.second == "keep-alive"){
            m_linger = true;
        }
        else if(kv.first == "Content-Length:"){
            m_content_length = atol(kv.second.c_str());
        }
        else if(kv.first == "Host:"){
            strcpy(m_host,kv.second.c_str());
        }
        /*
        else{
            LOG_INFO("oop!unknow header: %s", text);
        }
        */
    }
    if(m_content_length != 0){
        std::cout<<"m_content_length = "<<m_content_length<<std::endl;
        m_check_state = CHECK_STATE_CONTENT;
        return NO_REQUEST;
    }
    return GET_REQUEST;
}
HTTPHandler::HTTPCODE HTTPHandler::parse_content(std::string text){
    std::stringstream ss(text);
    std::string str;
    std::vector<std::string> d;
    //std::cout<<"parse_content"<<std::endl;
    //user=root&password=root
    while(getline(ss,str,'&')){
        //std::cout<<"str="<<str<<std::endl;
        d.push_back(str);
    }
    for(auto dd:d){
        std::stringstream ss1(dd);
        std::string str1;
        std::vector<std::string> ds;
        while(getline(ss1,str1,'=')){
            ds.push_back(str1);
            //std::cout<<"str1 = "<<str1<<std::endl;
        }
        //std::cout<<"datamap["<<ds[0]<<"] = "<<ds[1]<<std::endl;
        datamap[ds[0]] = ds[1];
    }
    return GET_REQUEST;
}
HTTPHandler::HTTPCODE HTTPHandler::do_request(){
    strcpy(m_real_file, doc_root);
    std::cout<<"do_request:m_url = "<<m_url<<std::endl;
    int len = strlen(doc_root);
    const char *p = strrchr(m_url, '/');
    if(cgi == 1 && ( *(p+1) == '2' || *(p+1) == '3' ) ){
        std::cout<<"m_url = "<<m_url<<std::endl;
        int flag = *(p+1) - '0';
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/");
        strcat(m_url_real, m_url + 2);
        strncpy(m_real_file + len, m_url_real, FILENAME_LEN - len - 1);
        free(m_url_real);
        char name[100], password[100];
        strcpy(name, datamap[userkey].c_str());
        strcpy(password, datamap[passwordkey].c_str());
        //RPC相关
        //演示调用远程发布的rpc方法login
        ik::UserServiceRpc_Stub stub(new RpcChannel());
        //请求参数和响应
        ik::LoginRequest request;
        //设置姓名、密码
        switch(flag){
        case 2:{
            request.set_name(name);
            request.set_password(password);
            //step 1.用redis查一下
            DAO::pUser* user = new DAO::pUser();
            user->set_username(name);
            user->set_password(password);
            DAO::pUser* quser = new DAO::pUser();
            redisUtils->HGETALL("service1",name,*user);
            if(quser->username()==name&&quser->password()==password){
                strcpy(m_url, "/welcome.html");
                break;
            }
            //step 2.无结果，用RPC查一下
            //step 3.登陆成功，封装到redis里
            ik::LoginResponse response;
            
            //发起rpc调用，等待响应返回
            stub.Login(nullptr, &request, &response, nullptr);
            //rpc调用完成，读调用的结果
            if (response.errmsg().error() == 0){
                //没错误
                cout << "rpc login response: " << response.success() << endl;
                if(response.success()){
                    redisUtils->HSMSET("service0",name,user);
                    strcpy(m_url, "/welcome.html");
                }
                else 
                    strcpy(m_url, "/logError.html");
            }
            else{
                //有错误
                cout << "rpc login response errer: " << response.errmsg().error_msg() << endl;
                strcpy(m_url, "/logError.html");
            }
            break;
        }
        case 3:{
            ik::RegisterRequest reg_request;
            reg_request.set_id(0);
            reg_request.set_name(name);
            reg_request.set_password(password);
            ik::RegisterResponse reg_response;

            RpcControl control;

            stub.Register(&control,&reg_request,&reg_response,nullptr);
            //rpc调用完成，读调用的结果
            if (reg_response.error().error() == 0){
                //没错误
                cout << "rpc login response: " << reg_response.success() << std::endl;
                if(reg_response.success())
                    strcpy(m_url, "/log.html");
                else 
                    strcpy(m_url, "/registerError.html");
            }
            else{
                //有错误
                std::cout << "rpc login response errer: " << reg_response.error().error_msg() << endl;
                strcpy(m_url, "/registerError.html");
            }
            break;
        }
        default:
            break;
        }
        strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);
        std::cout<<"m_real_file = "<<m_real_file<<std::endl;
        std::cout<<"m_url = "<<m_url<<std::endl;
    }
    else if(*(p+1) == '0'){
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/register.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if(*(p+1) == '1'){
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/log.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if(*(p+1) == '5'){
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/picture.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);  
    }
    else if(*(p+1) == '6'){
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/video.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else if (*(p + 1) == '7')
    {
        char *m_url_real = (char *)malloc(sizeof(char) * 200);
        strcpy(m_url_real, "/fans.html");
        strncpy(m_real_file + len, m_url_real, strlen(m_url_real));

        free(m_url_real);
    }
    else
        strncpy(m_real_file + len, m_url, FILENAME_LEN - len - 1);
    //通过stat获取请求资源文件信息，成功则将信息更新到m_file_stat结构体
    //失败返回NO_RESOURCE状态，表示资源不存在
    if (stat(m_real_file, &m_file_stat) < 0)
        return NO_RESOURCE;
    //判断文件的权限，是否可读，不可读则返回FORBIDDEN_REQUEST状态
    if (!(m_file_stat.st_mode & S_IROTH))
        return FORBIDDEN_REQUEST;
    //判断文件类型，如果是目录，则返回BAD_REQUEST，表示请求报文有误
    if (S_ISDIR(m_file_stat.st_mode))
        return BAD_REQUEST;
    //以只读方式获取文件描述符，通过mmap将该文件映射到内存中
    int fd = open(m_real_file, O_RDONLY);
    //用于将一个文件或其他对象映射到内存，提高文件的访问速度。
    m_file_address = (char *)mmap(0, m_file_stat.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    //避免文件描述符的浪费和占用
    close(fd);
    //表示请求文件存在，且可以访问
    return FILE_REQUEST;
}
bool HTTPHandler::add_response(const char *format, ...){
    if(m_write_idx >= WRITE_BUFFER_SIZE)
        return false;
    va_list arg_list;
    va_start(arg_list, format);
    int len = vsnprintf(m_write_buf + m_write_idx, WRITE_BUFFER_SIZE - 1 - m_write_idx, format, arg_list);
    if(len >= (WRITE_BUFFER_SIZE - 1 - m_write_idx)){
        va_end(arg_list);
        return false;
    }
    m_write_idx += len;
    va_end(arg_list);
    //LOG_INFO("request:%s", m_write_buf);
    return true;
}
bool HTTPHandler::add_status_line(int status, const char *title){
    return add_response("%s %d %s\r\n", "HTTP/1.1", status, title);
}
bool HTTPHandler::add_content_length(int content_len){
    return add_response("Content-Length:%d\r\n", content_len);
}
bool HTTPHandler::add_content_type(){
    return add_response("Content-Type:%s\r\n", "text/html");
}
bool HTTPHandler::add_linger()
{
    return add_response("Connection:%s\r\n", (m_linger == true) ? "keep-alive" : "close");
}
bool HTTPHandler::add_blank_line(){
    return add_response("%s", "\r\n");
}
bool HTTPHandler::add_content(const char* content){
    return add_response("%s", content);
}
bool HTTPHandler::add_headers(int content_len){
    return add_content_length(content_len) && add_linger() && add_blank_line();
}
bool HTTPHandler::process_write(HTTPCODE ret){
    switch(ret){
    case INTERNAL_ERROR:{
        //TODO
        add_status_line(500, error_500_title);
        add_headers(strlen(error_500_form));
        if(!add_content(error_500_form))
            return false;
        break;
    }
    case BAD_REQUEST:{
        //TODO
        add_status_line(404, error_404_title);
        add_headers(strlen(error_404_form));
        if(!add_content(error_404_form))
            return false;
        break;
    }
    case FORBIDDEN_REQUEST:{
        //TODO
        add_status_line(403, error_403_title);
        add_headers(strlen(error_403_form));
        if(!add_content(error_403_form))
            return false;
        break;
    }
    case FILE_REQUEST:{
        //TODO
        add_status_line(200, ok_200_title);
        if(m_file_stat.st_size != 0){
            add_headers(m_file_stat.st_size);
            m_iv[0].iov_base = m_write_buf;
            m_iv[0].iov_len = m_write_idx;
            m_iv[1].iov_base = m_file_address;
            m_iv[1].iov_len = m_file_stat.st_size;
            m_iv_count = 2;
            bytes_to_send = m_write_idx + m_file_stat.st_size;
            return true;
        }
        else{
            const char *ok_string = "<html><body></body></html>";
            add_headers(strlen(ok_string));
            if(!add_content(ok_string))
                return false;
        }
        break;
    }
    default:
        return false;
    }
    m_iv[0].iov_base = m_write_buf;
    m_iv[0].iov_len = m_write_idx;
    m_iv_count = 1;
    bytes_to_send = m_write_idx;
    return true;
}
void HTTPHandler::process(){
    //read
    m_checked_idx = 0;
    std::cout<<"process read is start!"<<std::endl;
    HTTPCODE read_ret = process_read();
    if(read_ret == NO_REQUEST){
        if (g_reactor->RegisterHandler(this, reactor::kReadEvent) != 0){
            printf("error: register handler failed\n");
            //delete handler;
        }
        return;
    }
    if (g_reactor->RegisterHandler(this, reactor::kWriteEvent) != 0){
        printf("error: register handler failed\n");
        //delete handler;
    }
    //write
    bool write_ret = process_write(read_ret);
    if(!write_ret){
        //close
    }
}