#pragma once

#include "Socket.h"
#include "ManagerSelector.h"
#include "utils.h"

#include <vector>
//#include <thread>

namespace kikilib
{
	class EventManager;
	class EventServiceFactory;
	class ThreadPool;

	//�¼�������
	//ְ��
	//1�������������е��¼�������
	//2�������µ��������ӣ������µ��¼�����ʵ�壬Ȼ�����ManagerSelector��ѡ�����ѡ���������¼��Ĺ�����
	//3�������̳߳ع���ʵ��
	//��Ҫ�����������������
	//1���̳߳ع���threadpool
	//2���¼�������eventmanager
	class EventMaster
	{
	public:
		EventMaster(EventServiceFactory* pEvServeFac);

		~EventMaster();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventMaster);

		//�������EventManager�߳�Ȼ��ѭ��accept
		//mgrCnt : �����¼���������������һ���¼���������Ӧһ���߳�
		//listenPort : Ҫ���ĸ��˿���ѭ��listen
		void Loop(int mgrCnt, int listenPort);

		void Stop() { _stop = true; }

	private:

		bool _stop;

		Socket _listener;

		//std::thread* _acceptor;

		//�����¼�����Ĺ���
		EventServiceFactory* _pEvServeFac;

		//�̳߳�
		ThreadPool* _pThreadPool;

		//�¼���������ѡ����������ѡ����һ���¼����ĸ��¼�����������
		ManagerSelector _mgrSelector;

		//�¼��������б�
		std::vector<EventManager*> _evMgrs;
	};

}
