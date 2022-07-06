#include <errno.h>
#include <assert.h>
#include <vector>
#include <fcntl.h>
#include <time.h>
#include "event_demultiplexer.h"

namespace reactor
{
    EpollDemultiplexer::EpollDemultiplexer()
    {
        m_epoll_fd = ::epoll_create(FD_SETSIZE);
        assert(m_epoll_fd != -1);
        m_fd_num = 0;
        m_pool = new ThreadPool::ThreadPool(4);
        m_pool->init();
    }

    EpollDemultiplexer::~EpollDemultiplexer()
    {
        ::close(m_epoll_fd);
    }

    int EpollDemultiplexer::Work(
            std::unordered_map<handler_t, EventHandler *>  handlers,
            int timeout, time_heap* event_timer)
    {
        std::vector<epoll_event> ep_evts(m_fd_num);
        int num = epoll_wait(m_epoll_fd, &ep_evts[0], ep_evts.size(), timeout);
        if (num > 0)
        {
            //std::cout<<"num = "<<num<<std::endl;
            for (int idx = 0; idx < num; ++idx)
            {
                handler_t handle = ep_evts[idx].data.fd;
                if(handlers.find(handle) == handlers.end()){
                    std::cout<<"Not Found!"<<std::endl;
                }
                assert(handlers.find(handle) != handlers.end());
                if ((ep_evts[idx].events & EPOLLERR) ||
                        (ep_evts[idx].events & EPOLLHUP))
                {
                    //(*handlers)[handle]->HandleError();
                    auto Proc = std::bind(&EventHandler::HandleError, handlers[handle]);
                    m_pool->submit(Proc);
                }
                else
                {
                    if (ep_evts[idx].events & EPOLLIN)
                    {
                        //(*handlers)[handle]->HandleRead();
                        auto Proc = std::bind(&EventHandler::HandleRead, handlers[handle]);
                        if(handle != listen_handle)m_pool->submit(Proc);
                        else{
                            //std::cout<<"handle "<<handle<<" ready for read event!"<<std::endl;
                            handlers[handle]->HandleRead();
                        }
                    }
                    if (ep_evts[idx].events & EPOLLOUT)
                    {
                        //(*handlers)[handle]->HandleWrite();
                        auto Proc = std::bind(&EventHandler::HandleWrite, handlers[handle]);
                        m_pool->submit(Proc);
                    }
                }
            }
        }
        if (event_timer != NULL)
        {
            event_timer->tick();
        }

        return num;
    }

    /* 设置文件描述符为非阻塞 */
    int EpollDemultiplexer::setNonBlocking(handler_t sockfd){
	    // 先获取fd的属性再和非阻塞属性做或运算
	    if (-1 == fcntl(sockfd, F_SETFL, fcntl(sockfd, F_GETFD, 0)|O_NONBLOCK)){
		    return -1;
	    }
	    return 0;
    }
    int EpollDemultiplexer::RequestEvent(handler_t handle, event_t evt,int flag)
    {
        if(flag == 1){
            listen_handle = handle;
        }
        epoll_event ep_evt;
        ep_evt.data.fd = handle;
        ep_evt.events = 0;

        if (evt & kReadEvent)
        {
            //std::cout<<"handle "<<handle<<" register read!"<<std::endl;
            ep_evt.events |= EPOLLIN;
        }
        if (evt & kWriteEvent)
        {
            //std::cout<<"handle "<<handle<<" register write"<<std::endl;
            ep_evt.events |= EPOLLOUT;
        }
        if(flag!=1) ep_evt.events |= EPOLLONESHOT;
        //ep_evt.events |= EPOLLET;

        setNonBlocking(handle);   //将文件描述符设置成非阻塞

        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, handle, &ep_evt) != 0)
        {
            if (errno == ENOENT)
            {
                if (epoll_ctl(m_epoll_fd, EPOLL_CTL_ADD, handle, &ep_evt) != 0)
                {
                    return -errno;
                }
                ++m_fd_num;
                //std::cout<<time(NULL)<<std::endl;
                //std::cout<<"Epoll ctl : handler = "<<handle<<std::endl;
            }
        }
        return 0;
    }

    int EpollDemultiplexer::UnrequestEvent(handler_t handle)
    {
        epoll_event ep_evt;
        if (epoll_ctl(m_epoll_fd, EPOLL_CTL_DEL, handle, &ep_evt) != 0)
        {
            return -errno;
        }
        --m_fd_num;
        return 0;
    }
} // namespace reactor

