//@Author Liu Yukang 
#pragma once
#include "Time.h"
#include "Socket.h"
#include "utils.h"

#include <map>
#include <vector>
#include <mutex>
#include <functional>

namespace kikilib
{
	//��ʱ��
	//ְ��
	//1������ ĳһʱ�̡�����ʱ����Ҫִ�еĺ��� ��ӳ���
	//2���ṩ��ʱ�¼��Ĳ����ӿ�
	//3�����಻��֤�̰߳�ȫ���̰߳�ȫ��EventManager����
	class Timer
	{
	public:
		Timer(Socket& timeSock) : _timeSock(timeSock) {}
		Timer(Socket&& timeSock) : _timeSock(timeSock) {}
		~Timer() {}

		DISALLOW_COPY_MOVE_AND_ASSIGN(Timer);

		//��ȡ�����Ѿ���ʱ����Ҫִ�еĺ���
		void getExpiredTask(std::vector<std::function<void()>> &tasks);

		//��timeʱ����Ҫִ�к���cb
		void runAt(Time time, std::function<void()>& cb);
		void runAt(Time time, std::function<void()>&& cb);

	private:
		//��timefd��������ʱ�䣬time�Ǿ���ʱ��
		void resetTimeOfTimefd(Time time);

		Socket _timeSock;

		//��ʱ���ص���������
		std::map<Time, std::function<void()>> _timerCbMap;
	};

}
