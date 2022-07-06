#ifndef REACTOR_REACTOR_H_
#define REACTOR_REACTOR_H_
#include "singleton.h"
#include "EventHandler.h"
#include "ThreadPool.h"
#include "timeheap.h"
namespace reactor{
    typedef unsigned int event_t;
    enum
    {
        kReadEvent    = 0x01,
        kWriteEvent   = 0x02,
        kErrorEvent   = 0x04,
        kEventMask    = 0xff
    };
    class ReactorImplementation;   //implemation class 
    //handler
    class Reactor{
        DECLARE_SINGLETON_CLASS(Reactor);
    public:
        Reactor();
        ~Reactor();
        //注册handler
        virtual int RegisterHandler(EventHandler* handler, event_t evt, int flag=0);
        //移除handler
        virtual int RemoveHandler(EventHandler* handler);
        //Reactor开始工作
        virtual void Work();
        virtual int RegisterTimerTask(heap_timer* timerevent);
    private:
        //Reactor是单例模式，不允许拷贝（这里拷贝指的构造）
        Reactor(const Reactor &);
        //Reactor不允许赋值
        Reactor & operator=(const Reactor &);
    private:
        ReactorImplementation * m_reactor_impl;
    };
}

#endif