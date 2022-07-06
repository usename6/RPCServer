#include "reactor.h"
#include "event_demultiplexer.h"
#include <unordered_map>
#include <time.h>
#define INITSIZE 10
namespace reactor{
    class ReactorImplementation{
    public:
        ReactorImplementation(){
            m_demultiplexer = new EpollDemultiplexer();
            m_eventtimer = new time_heap(INITSIZE);
        }
        ~ReactorImplementation(){
        }
        int RegisterHandler(EventHandler* handler, event_t event,int flag){
            reactor::handler_t handle = handler->GetHandle();
            auto it = this->m_handlers.find(handle);
            if (it == this->m_handlers.end()){
                this->m_handlers[handle] = handler;
            }
            return m_demultiplexer->RequestEvent(handle, event, flag);
        }
        int RemoveHandler(EventHandler *handler){
            handler_t handle = handler->GetHandle();
            auto it = this->m_handlers.find(handle);
            if (it == this->m_handlers.end()){
                return -1;
            }
            this->m_handlers.erase(handle);
            return m_demultiplexer->UnrequestEvent(handle);
        }
        //注册定时任务
        int RegisterTimerTask(heap_timer* timerevent)
        {
            if (timerevent == NULL)
                return -1;
            m_eventtimer->add_timer(timerevent);
            return 0;
        }
        void Work(){
            //应该启动Epoll EventHandler工作线程开启
            int timeout = 0;
            if (m_eventtimer->top() == NULL){
                timeout = 0;
            }
            else{
                timeout = ((m_eventtimer->top())->expire - time(NULL)) * 1000;
            }
            this->m_demultiplexer->Work(m_handlers, timeout, this->m_eventtimer);   
        }
    private:
        std::unordered_map<handler_t, EventHandler *> m_handlers;
        EventDemultiplexer * m_demultiplexer;  //事件循环处理器
        time_heap* m_eventtimer;
    };
    Reactor::Reactor(){
        m_reactor_impl = new ReactorImplementation();
    }

    Reactor::~Reactor(){
        delete m_reactor_impl;
    }

    int Reactor::RegisterHandler(EventHandler * handler, event_t evt,int flag){
        return m_reactor_impl->RegisterHandler(handler, evt, flag);
    }

    int Reactor::RemoveHandler(EventHandler * handler){
        return m_reactor_impl->RemoveHandler(handler);
    }

    void Reactor::Work(){
        m_reactor_impl->Work();
    }

    int Reactor::RegisterTimerTask(heap_timer* timerevent){
        return m_reactor_impl->RegisterTimerTask(timerevent);
    }
}