//@Author Liu Yukang 
#pragma once
#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <atomic>
#include <condition_variable>
#include <functional>
#include <stdexcept>

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
		void enqueue(std::function<void()>&&);
		
		//��������
		DISALLOW_COPY_MOVE_AND_ASSIGN(ThreadPool);

	private:
		//��ǰ���������м�������Ķ����±�
		int _usableQue;
		bool _stop;

		// need to keep track of threads so we can join them
		std::vector< std::thread > _workers;
		// ˫������У�һ��һֱ�ڱ���������һ��һֱ�ڱ��߳�����ִ������
		std::queue< std::function<void()> > _tasks[2];

		// synchronization
		std::mutex _threadsMutex;//ִ��������߳�֮�����
		std::mutex _changeQueMutex;//����Ҫ�л�������У�ʹ�ô���
		std::condition_variable _condition;
	};
}