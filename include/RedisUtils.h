#ifndef REDISUTILS_H_
#define REDISUTILS_H_
#include <typeinfo>
#include <cxxabi.h>
#include <hiredis/hiredis.h> 
#include <sstream>
#include <cstdio>
#include <string>
#include <cstring>
#include <iostream>
#include <vector>
#include <google/protobuf/message.h>
#include <google/protobuf/service.h>
#include <google/protobuf/reflection.h>
#include <google/protobuf/descriptor.h>
#include <google/protobuf/descriptor.pb.h>
#include <google/protobuf/dynamic_message.h>
#include <google/protobuf/compiler/importer.h>
#include "pUser.pb.h"
const std::string stringname="std::__cxx11::basic_string<char, std::char_traits<char>, std::allocator<char> >";	

const char* TypeToName(const char* name);
namespace Impl {
    template <typename T>
    struct B { 
    public:
        B(){}
        T value;
    };
    template <>
    struct B<int> { 
    public:
        B(){}
        bool operator()(std::string str){
            value = atoi(str.c_str());
        }
        int value;
    };
    template <>
    struct B<double>{
    public:
        B(){}
        bool operator()(std::string str){
            value = atof(str.c_str());
        }
        double value;
    };
    template <>
    struct B<std::string>{
    public:
        B(){}
        bool operator()(std::string str){
            value = str;
        }
        std::string value;
    };
}
class RedisUtils{
public:
    RedisUtils();
    ~RedisUtils();
    //启动redis
    void start();
    template<typename T>
    inline std::string trans2str(T object){
        std::stringstream ss;
        ss<<object;
        return ss.str();
    }
    template<typename T>
    inline void parseData(T& value, std::string str){
        Impl::B<T> trans;
        trans(str);
        value = trans.value; 
    }
    template<typename I,typename O>
    void getSingleValueByKey(std::string name, I key, O& value){
        std::string keystr = name + ":" +trans2str(key);
        std::string command = "get " + keystr;
        redisReply* r = (redisReply*)redisCommand(c, command.c_str()); 
        std::cout<<TypeToName(typeid(value).name())<<std::endl;
        if ( r->type != REDIS_REPLY_STRING) { 
            printf("Failed to execute command[%s]\n",command.c_str()); 
            freeReplyObject(r); 
            redisFree(c); 
            return; 
        }
        parseData(value, r->str);
        //std::cout<<"value = "<<value<<std::endl;
        freeReplyObject(r); 
    }
    template<typename I,typename O>
    void getListsByKey(std::string name, I key, std::vector<O>& values,int left=0,int right=0){
        std::string keystr = name + ":" + trans2str(key);
        std::string leftstr = trans2str(left);
        std::string rightstr = trans2str(right);
        std::string command = "LRANGE " + keystr + " " + leftstr + " " + rightstr;
        redisReply* r = (redisReply*)redisCommand(c, command.c_str()); 
        if ( r->type != REDIS_REPLY_ARRAY) { 
            printf("Failed to execute command[%s]\n",command.c_str()); 
            freeReplyObject(r); 
            redisFree(c); 
            return; 
        }
        for(int i=0;i<r->elements;i++){
            O tvalue;
            //std::cout<<"r->str"<<r->element[i]->str<<std::endl;
            parseData(tvalue, r->element[i]->str);
            values.emplace_back(tvalue);
        }
        freeReplyObject(r);
    }
    template<typename I,typename O>
    bool HGETALL(std::string name,I key,O& object){
        std::string keystr = name + ":" +trans2str(key);
        std::string command = "HGETALL " + keystr;
        std::cout<<command<<std::endl;
        redisReply* r = (redisReply*)redisCommand(c, command.c_str()); 
        if ( r->type != REDIS_REPLY_ARRAY) { 
            printf("Failed to execute command[%s]\n",command.c_str()); 
            freeReplyObject(r); 
            redisFree(c); 
            return false; 
        }
        const google::protobuf::Descriptor* descriptor = google::protobuf::DescriptorPool::generated_pool()->FindMessageTypeByName("DAO.User");
        if(descriptor == NULL){
            //std::cout<<"yes"<<std::endl;
            return false;
        }
        //利用Descriptor拿到类型注册的instance. 这个是不可修改的.
        const google::protobuf::Message * prototype = google::protobuf::MessageFactory::generated_factory()->GetPrototype(descriptor);
        // 构造一个可用的消息.
        google::protobuf::Message * instance = prototype->New(); //创建新的 person message对象。
        auto ref = instance->GetReflection();
        for(int i=0;i+1<r->elements;i+=2){
            //auto reflecter = i;//2.1通过字符串"Person"取得的实例instance
            const google::protobuf::FieldDescriptor* field = descriptor->FindFieldByName(r->element[i]->str); // Person这个Message 中有name字段
            if (!field->is_repeated()) {
                //获取数据
                switch (field->cpp_type()){
                case google::protobuf::FieldDescriptor::CppType::CPPTYPE_INT32:{
                    int value1 = ref->GetInt32(*instance, field);
                    parseData(value1, r->element[i+1]->str);
                    ref->SetInt32(instance, field, value1) ; //反射来设置name字段
                    break;
                }
                case google::protobuf::FieldDescriptor::CppType::CPPTYPE_STRING:{
                    std::string value2 = ref->GetString(*instance, field);
                    std::string str = r->element[i+1]->str;
                    str = str.substr(1,str.size()-2);
                    parseData(value2, str);
                    ref->SetString(instance, field, value2) ; //反射来设置name字段
                    break;
                }
                case google::protobuf::FieldDescriptor::CppType::CPPTYPE_DOUBLE:{
                    double value3 = ref->GetDouble(*instance, field);
                    parseData(value3, r->element[i+1]->str);
                    ref->SetDouble(instance, field, value3) ; //反射来设置name字段
                    break;
                }
                }
            }
            else {
                std::cout<<"error"<<std::endl;
            }
        }
        std::cout<<TypeToName(typeid(object).name())<<std::endl;
        O* t = (O *)instance;
        object = (*t);
        return true;
    }
    template<typename I, typename O>
    bool setSingleValue(std::string name,I key,O value){
        std::string keystr = name + ":" +trans2str(key);
        std::string valuestr = trans2str(value);
        std::string command = "set " + keystr + " " + valuestr;
        redisReply* r = (redisReply*)redisCommand(c, command.c_str()); 
        if( NULL == r) { 
            printf("Execut command1 failure\n"); 
            redisFree(c); 
            return false; 
        } 
        if( !(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK")==0)) { 
            printf("Failed to execute command[%s]\n",command.c_str()); 
            freeReplyObject(r); 
            redisFree(c); 
            return false; 
        }   
        freeReplyObject(r);
        return true;
    }
    template<typename I,typename O>
    bool LPUSHList(std::string name,I key,O value){
        std::string keystr = name + ":" +trans2str(key);
        std::string valuestr = trans2str(value);
        std::string command = "LPUSH " + keystr + " " + valuestr;
        redisReply* r = (redisReply*)redisCommand(c, command.c_str()); 
        if( NULL == r) { 
            printf("Execut command1 failure\n"); 
            redisFree(c); 
            return false; 
        } 
        if( !(r->type == REDIS_REPLY_INTEGER )) { 
            printf("Failed to execute command[%s]\n",command.c_str()); 
            freeReplyObject(r); 
            redisFree(c); 
            return false; 
        }   
        std::cout<<"r->integer = "<<r->integer<<std::endl;
        freeReplyObject(r);
        return true;
    } 
    template<typename I>
    bool HSMSET(std::string name,I key,google::protobuf::Message* message){
        std::string keystr = name + ":" +trans2str(key);
        std::string op;
        const google::protobuf::Descriptor *des = message->GetDescriptor();
        const google::protobuf::Reflection *ref = message->GetReflection();
        int fieldCount = des->field_count();
        for (int i = 0; i < fieldCount; i++){
            std::string namestr;
            std::string valuestr;
            ///< get field with index i
            const google::protobuf::FieldDescriptor *field = des->field(i);
            bool flag=false;
            if (!field->is_repeated()) {
                //获取数据
                namestr = field->name();
                switch (field->cpp_type()){
                case google::protobuf::FieldDescriptor::CppType::CPPTYPE_INT32:{
                    int value1 = ref->GetInt32(*message, field);
                    valuestr = trans2str(value1);
                    break;
                }
                case google::protobuf::FieldDescriptor::CppType::CPPTYPE_STRING:{
                    std::string value2 = ref->GetString(*message, field);
                    valuestr = trans2str(value2);
                    flag = true;
                    break;
                }
                case google::protobuf::FieldDescriptor::CppType::CPPTYPE_DOUBLE:{
                    double value3 = ref->GetDouble(*message, field);
                    valuestr = trans2str(value3);
                    break;
                }
                }
            }
            if(flag)op = op + " " + namestr + " " + "\"" + valuestr + "\"";
            else op = op + " " + namestr + " " + valuestr;
        }
        std::string command = "HMSET " + keystr + op;
        std::cout<<command<<std::endl;
        redisReply* r = (redisReply*)redisCommand(c, command.c_str()); 
        if( NULL == r) { 
            printf("Execut command1 failure\n"); 
            redisFree(c); 
            return false; 
        } 
        if( !(r->type == REDIS_REPLY_STATUS && strcasecmp(r->str,"OK")==0)) { 
            printf("Failed to execute command[%s]\n",command.c_str()); 
            freeReplyObject(r); 
            redisFree(c); 
            return false; 
        }   
        freeReplyObject(r);
        return true;
    }
private:
    redisContext* c;
};
#endif