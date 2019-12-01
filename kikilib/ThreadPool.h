#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <future>
#include <functional>
#include <stdexcept>
#include <signal.h>

#include "LogManager.h"
#include "Parameter.h"
#include "utils.h"

namespace kikilib
{
	//�̳߳�
	//˫������У�һ��һֱ�������߼�������һ��һֱ�ڱ��̳߳��е��߳�����ִ������
	class ThreadPool {
	public:
		ThreadPool();
		~ThreadPool();

		//���������
		template<class F, class... Args>
		auto enqueue(F&& f, Args&&... args)
			->std::future<typename std::result_of<F(Args...)>::type>;
		
		//��������
		DISALLOW_COPY_MOVE_AND_ASSIGN(ThreadPool);

	private:
		// need to keep track of threads so we can join them
		std::vector< std::thread > workers;
		// ˫������У�һ��һֱ�ڱ���������һ��һֱ�ڱ��߳�����ִ������
		std::queue< std::function<void()> > tasks[2];

		// synchronization
		std::mutex _threadsMutex;//ִ��������߳�֮�����
		std::mutex _changeQueMutex;//����Ҫ�л�������У�ʹ�ô���
		std::condition_variable condition;

		//��ǰ���������м�������Ķ����±�
		int _usableQue;
		bool stop;
	};

	// the constructor just launches some amount of workers
	inline ThreadPool::ThreadPool()
		: stop(false)
	{
		_usableQue = 0;
		for (size_t i = 0; i < Parameter::threadPoolCnt; ++i)
		{
			workers.emplace_back(
				[this]
				{
					while (true)
					{
						std::function<void()> task;

						{
							std::unique_lock<std::mutex> lock(this->_threadsMutex);
							this->condition.wait(lock,
								[this] { return this->stop || !this->tasks[_usableQue].empty() || !this->tasks[!_usableQue].empty(); });

							if (this->stop && this->tasks[_usableQue].empty() && this->tasks[!_usableQue].empty())
							{
								return;
							}
							int taskQue = !_usableQue;
							if (this->tasks[taskQue].empty())
							{//�����Ŀ���Ǿ����ܼ��ٶ������̵�����
								this->_changeQueMutex.lock();
								_usableQue = taskQue;
								this->_changeQueMutex.unlock();
							}
							taskQue = !_usableQue;
							try
							{
								task = std::move(this->tasks[taskQue].front());
								this->tasks[taskQue].pop();
							}
							catch (std::exception & e)
							{
								RecordLog(e.what());
							}

						}
						try
						{
							task();
						}
                        catch (std::exception & e)
                        {
                            RecordLog(e.what());
                        }

					}
				});
		}
	}

	// add new work item to the pool
	template<class F, class... Args>
	auto ThreadPool::enqueue(F&& f, Args&&... args)
		-> std::future<typename std::result_of<F(Args...)>::type>
	{
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared< std::packaged_task<return_type()> >(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
			);

		std::future<return_type> res = task->get_future();

		// don't allow enqueueing after stopping the pool
		if (stop)
        {
            RecordLog(ERROR_DATA_INFORMATION, "unknown err in GetActServ()!");
            throw("error : unknown err in GetActServ()!");
        }

		{
			std::unique_lock<std::mutex> changeLock(_changeQueMutex);
			tasks[_usableQue].emplace([task]() { (*task)(); });
		}
		condition.notify_one();
		return res;
	}

	// the destructor joins all threads
	inline ThreadPool::~ThreadPool()
	{
		{
			std::unique_lock<std::mutex> lock(_threadsMutex);
			stop = true;
		}
		condition.notify_all();
		for (std::thread& worker : workers)
			worker.join();
	}

}