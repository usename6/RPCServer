#ifndef EventHandler_H
#define EventHandler_H
#include "AbstractProducerFactory.h"
namespace reactor{
    typedef int handler_t;
    class EventHandler{
    public:
        virtual handler_t GetHandle()=0;
        virtual void HandleRead(){}
        virtual void HandleWrite(){}
        virtual void HandleError(){}
        virtual void setHandle(handler_t thandle){}
        //virtual void process(){}
    protected:
        EventHandler(){}
        virtual ~EventHandler(){}
    };
}

#endif