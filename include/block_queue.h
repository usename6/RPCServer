#ifndef BLOCK_QUEUE_H
#define BLOCK_QUEUE_H

#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <sys/time.h>
#include <vector>
#include <condition_variable>     // std::condition_variable
#include <mutex>
template<class T>
class block_queue{
public:
    block_queue(int max_size = 1000){
        if(max_size <= 0){
            exit(-1);
        }

        m_max_size = max_size;
        m_size = 0;
    }
    void deleteFunc(){
        m_array.clear();
    }
    ~block_queue(){
        m_mutex.lock();
        deleteFunc();
        m_mutex.unlock();
    }
    void clear(){
        m_mutex.lock();
        m_size = 0;
        deleteFunc();
        m_mutex.unlock();
    }
    bool full(){
        m_mutex.lock();
        if(m_size >= m_max_size){
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }
    bool empty(){
        m_mutex.lock();
        if(0 == m_size){
            m_mutex.unlock();
            return true;
        }
        m_mutex.unlock();
        return false;
    }
    T front(){
        m_mutex.lock();
        if(0 == m_size){
            m_mutex.unlock();
            return NULL;
        }
        T value = m_array.front();
        m_mutex.unlock();
        return value;
    }
    T back(){
        m_mutex.lock();
        if(0 == m_size){
            m_mutex.unlock();
            return NULL;
        }
        T value = m_array.back();
        m_mutex.unlock();
        return value;
    }

    int size(){
        int tmp = 0;
        m_mutex.lock();
        tmp = m_size;
        m_mutex.unlock();
        return tmp;
    }

    int max_size(){
        int tmp = 0;
        m_mutex.lock();
        tmp = m_max_size;
        m_mutex.unlock();
    }

    bool push(const T& item){
        std::unique_lock<std::mutex> lk(m_mutex);
		spaceAvailable.wait(lk, [&] {
            return this->m_size < this->m_max_size;
		});
        m_array.push_back(item);
        /*for(auto ii:m_array){
            std::cout<<ii.aa<<" ";
        }*/
        //std::cout<<std::endl;
        std::cout<<"push:"<<m_array.back()<<std::endl;
        m_size++;
        lk.unlock();
        dataAvailable.notify_one();
        return true;
    }
    bool pop(T& item){
        std::unique_lock<std::mutex> lk(m_mutex);
        dataAvailable.wait(lk,[&]{
            return this->m_size > 0;
        });
        m_size--;
        item = m_array.front();
        m_array.erase(m_array.begin());
        std::cout<<"pop item:"<<item<<std::endl;
        lk.unlock();
        std::cout<<"successful pop 1:"<<std::endl;
        spaceAvailable.notify_one();
        std::cout<<"successful pop:"<<std::endl;
        return true;
    }
    bool pop(T& item,int ms_timeout){
        int ptime = time(NULL);
        bool flag=false;
        std::unique_lock<std::mutex> lk(m_mutex);
        dataAvailable.wait(lk,[&]{
            int ctime = time(NULL);
            if(ctime - ptime >= ms_timeout){
                flag=true;
            }
            return flag || this->m_size > 0;
        });
        if(flag){
            lk.unlock();
            return false;
        }
        m_size--;
        item = m_array.front();
        lk.unlock();
        spaceAvailable.notify_one();
        return true;
    }
private:
    std::mutex m_mutex;
    std::vector<T> m_array;
    int m_size;
    int m_max_size;
    std::condition_variable dataAvailable;
	std::condition_variable spaceAvailable;
};

#endif