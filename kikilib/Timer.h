//@Author Liu Yukang 
#pragma once
#include "Time.h"
#include "Socket.h"
#include "utils.h"

#include <map>
#include <mutex>
#include <functional>

namespace kikilib
{
	//��ʱ��
	//ְ��
	//1������ ĳһʱ�̡�����ʱ����Ҫִ�еĺ��� ��ӳ���
	//2���ṩ��ʱ�¼��Ĳ����ӿ�
	//3����Ҫ��֤�̰߳�ȫ����Ϊ���߳�ִ�е�HandleConnectionEvent������EventManager����ͬʱʹ��ͬһ����ʱ��
	class Timer
	{
	public:
		Timer(Socket timeFd) : _timeSock(timeFd) {}
		~Timer() {}

		DISALLOW_COPY_MOVE_AND_ASSIGN(Timer);

		//���������Ѿ���ʱ����Ҫִ�еĺ���
		void RunExpired();

		//��timeʱ����Ҫִ�к���cb
		void RunAt(Time time, std::function<void()> cb);

	private:
		Socket _timeSock;

		std::mutex _timerMutex;

		//��ʱ���ص���������
		std::multimap<Time, std::function<void()>> _timerCbMap;
	};

}
