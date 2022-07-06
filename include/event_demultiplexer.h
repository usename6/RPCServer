#ifndef REACTOR_EVENT_DEMULTIPLEXER_H_
#define REACTOR_EVENT_DEMULTIPLEXER_H_

#include <set>
#include <map>
#include <stdint.h>
#include <unistd.h>
#include <sys/epoll.h>
#include "ThreadPool.h"
#include "reactor.h"
namespace reactor
{
    //定义一个接口
    class EventDemultiplexer
    {
    public:
        //EventDemultiplexer的构造函数
        virtual ~EventDemultiplexer() {}
        //提交任务
        virtual int Work(std::unordered_map<handler_t, EventHandler *> handlers,
                                int timeout = 0, time_heap* event_timer=NULL) = 0;
        //注册监听事件
        virtual int RequestEvent(handler_t handle, event_t evt, int flag=0) = 0;
        //注销监听事件
        virtual int UnrequestEvent(handler_t handle) = 0;
    };

    ///////////////////////////////////////////////////////////////////////////////

    class EpollDemultiplexer : public EventDemultiplexer
    {
    public:
        EpollDemultiplexer();
        EpollDemultiplexer(int size);
        ~EpollDemultiplexer();
        int setNonBlocking(handler_t sockfd);
        virtual int Work(std::unordered_map<handler_t, EventHandler *> handlers,
                                int timeout = 0, time_heap* event_timer=NULL);
        //在Epoll中注册监听事件和监听的socket
        virtual int RequestEvent(handler_t handle, event_t evt, int flag=0);  
        //在Epoll中注销监听事件和监听的socket
        virtual int UnrequestEvent(handler_t handle);
    private:
        int  m_epoll_fd;
        int  m_fd_num;    //监听的数量
        ThreadPool::ThreadPool* m_pool;
        int pool_size;    //线程池线程的大小
        handler_t listen_handle = -1;
    };
} // namespace reactor

#endif // REACTOR_EVENT_DEMULTIPLEXER_H_

