#include "ThreadPool.h"
namespace ThreadPool {
	ThreadPool::ThreadPool() {

	}
	ThreadPool::ThreadPool(const int n_threads = 4)
		:m_threads(std::vector<std::thread>(n_threads)),
		m_shutdown(false)
	{
		//std::cout << "yes" << std::endl;
	}
	void ThreadPool::init() {
		for (int i = 0; i < m_threads.size(); i++) {
			m_threads.at(i) = std::thread(ThreadWorker(this, i));
		}
	}
	void ThreadPool::close() {
		m_shutdown = true;
		m_conditional_lock.notify_all();
		for (int i = 0; i < m_threads.size(); i++) {
			if (m_threads.at(i).joinable()) {
				m_threads.at(i).join();
			}
		}
	}
}