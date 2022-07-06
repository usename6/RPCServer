#ifndef Singleton_H
#define Singleton_H
#include <memory>
#include <mutex>
typedef std::mutex Mutex;
template<class T>
class Singleton{
public:
    static inline T* getInstance();
private:
    Singleton(void){}
    virtual ~Singleton(void){}
    Singleton(const Singleton&){}
    Singleton & operator=(const Singleton&){}
    //auto_ptr代理的单例
    static std::auto_ptr<T> SingleInstance;
    //保证该单例是线程安全
    static Mutex m_mutexInstance;
};

template<class T>
std::auto_ptr<T> Singleton<T>::SingleInstance;

template<class T>
Mutex Singleton<T>::m_mutexInstance;

template<class T>
inline T* Singleton<T>::getInstance(){
    if(SingleInstance.get()==NULL){
        m_mutexInstance.lock();
        if(SingleInstance.get()==NULL){
            std::auto_ptr<T> cur(new T());
            SingleInstance = cur;
        }
        m_mutexInstance.unlock();
    }
    return SingleInstance.get();
}

//Class that will implement the singleton mode,

//must use the macro in it's delare file
#define DECLARE_SINGLETON_CLASS( type ) \
private:\
    friend class std::auto_ptr< type >; \
    friend class Singleton< type >;

#endif // Singleton_H

