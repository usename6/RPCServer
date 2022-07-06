#ifndef THREADPOOL_H
#define THREADPOOL_H
#include <mutex>
#include <queue>
#include <functional>
#include <future>
#include <thread>
#include <utility>
#include <vector>
namespace ThreadPool {
	template <typename T>
	class SafeQueue {
	private:
		std::queue<T> m_queue;
		std::mutex m_mutex;
	public:
		SafeQueue();
		SafeQueue(SafeQueue&& other);
		~SafeQueue();
		bool empty();
		size_t size();
		void push(T& t);
		T top();
		bool pop(T& t);
	};
	//int add(int a, int b);
	class ThreadPool {
	private:
		class ThreadWorker {
		private:
			int m_id;
			ThreadPool* m_pool;
		public:
			ThreadWorker(ThreadPool* pool, const int id) :m_pool(pool), m_id(id) {

			}
			void operator()() {
				std::function<void()> func;
				bool dequeued;
				while (!m_pool->m_shutdown||m_pool->m_queue.size()!=0) {
					//printf("Thread %d is in!\n", m_id);
					//printf("shutdown cur state: %d", m_pool->m_shutdown);
					{
						std::unique_lock<std::mutex> lock(m_pool->m_conditional_mutex);
						//printf("Thread %d lock on!\n", m_id);
						if (m_pool->m_queue.empty()) {
							m_pool->m_conditional_lock.wait(lock);
						}
						//printf("Thread %d lock off\n", m_id);
						dequeued = m_pool->m_queue.pop(func);
					}
					if (dequeued) {
						//printf("Thread %d is start!\n",m_id);
						func();
					}
				}
				//printf("shutdown final state: %d", m_pool->m_shutdown);
			}
		};
		bool m_shutdown;
		SafeQueue<std::function<void()>> m_queue;
		std::vector<std::thread> m_threads;
		std::mutex m_conditional_mutex;
		std::condition_variable m_conditional_lock;
	public:
		ThreadPool();
		ThreadPool(const int n_threads);
		ThreadPool(const ThreadPool&) = delete;
		ThreadPool(ThreadPool&&) = delete;
		ThreadPool& operator=(const ThreadPool&) = delete;
		ThreadPool& operator=(ThreadPool&&) = delete;
		void init();
		void close();
		/*
		右值引用类型是独立于值的，一个右值引用参数作为函数的形参，
		在函数内部再转发该参数的时候它已经变成一个左值，并不是他原来的类型。
		需要一种方法能够按照参数原来的类型转发到另一个函数，这种转发类型称为完美转发。
		完美转发（Perfect Forwarding），是指在函数模板中，
		完全依照模板的参数的类型（即保持参数的左值、右值特征），
		将参数传递给函数模板中调用的另外一个函数。C++11中提供了这样的一个函数std::forward，它是为转发而生的，
		不管参数是T&&这种未定的引用还是明确的左值引用或者右值引用，它会按照参数本来的类型转发。
		*/
		template<typename F, typename... Args>
		auto submit(F&& f, Args &&... args) ->std::future<decltype(f(args...))> {
			std::function<decltype(f(args...))()> func = std::bind(std::forward<F>(f), std::forward<Args>(args)...);
			auto task_ptr = std::make_shared<std::packaged_task<decltype(f(args...))()>>(func);
			std::function<void()> warpper_func = [task_ptr]() {
				(*task_ptr)();
			};
			//std::cout << "submit success!" << std::endl;
			m_queue.push(warpper_func);
			//int sz = this->m_queue.size();
			//std::cout << "my queue size is " << sz << std::endl;
			m_conditional_lock.notify_one();
			return task_ptr->get_future();
		}
	};
	template<typename T>
	SafeQueue<T>::SafeQueue() {
	}
	template<typename T>
	SafeQueue<T>::SafeQueue(SafeQueue&& other) {
	}
	template<typename T>
	SafeQueue<T>::~SafeQueue() {
	}
	template<typename T>
	bool SafeQueue<T>::empty() {
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.empty();
	}
	template<typename T>
	size_t SafeQueue<T>::size() {
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.size();
	}
	template<typename T>
	void SafeQueue<T>::push(T& t) {
		std::unique_lock<std::mutex> lock(m_mutex);
		m_queue.emplace(t);
	}
	template<typename T>
	bool SafeQueue<T>::pop(T& t) {
		//printf("Pre: queue size: %d\n", m_queue.size());
		std::unique_lock<std::mutex> lock(m_mutex);
		if (m_queue.empty())
			return false;
		t = std::move(m_queue.front());
		m_queue.pop();
		//printf("After: queue size: %d\n", m_queue.size());
		return true;
	}
	template<typename T>
	T SafeQueue<T>::top() {
		std::unique_lock<std::mutex> lock(m_mutex);
		return m_queue.top();
	}
}
#endif