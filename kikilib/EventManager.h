#pragma once

#include "EventEpoller.h"
#include "Time.h"
#include "utils.h"

#include <set>
#include <vector>
#include <thread>
#include <mutex>
#include <functional>
#include <future>

namespace kikilib
{
	class EventService;
	class Timer;
	class ThreadPool;

	//�¼����ȼ�
	enum EventPriority
	{
		NORMAL_EVENT = 0,  //һ���¼�
		IMMEDIATE_EVENT,   //�����¼�
		EVENT_PRIORITY_TYPE_COUNT   //�¼����ȼ��������
	};

	//�¼�������
	//ְ��
	//1���ṩ�����в����¼����Ƴ����޸��¼��Ľӿ�
	//2���ṩ��ʱ����ʹ�ýӿ�
	//3��ѭ��ɨ���������¼��б�����Ķ���Ȼ������¼����͵�������غ���
	//��Ҫ�����������������
	//1��ѭ�����߳�looper
	//2���¼�������epoller
	//3����ʱ��timer
	//4�����е��¼�EventService
	class EventManager
	{
	public:
		EventManager(int idx, ThreadPool* threadPool);
		~EventManager();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventManager);

		//����һ���̣߳�Ȼ���߳���ѭ��ɨ���¼�
		void Loop();

		//���¼��������в���һ���¼�,�����̰߳�ȫ��
		void Insert(EventService* ev);

		//���¼����������Ƴ�һ���¼�,�����̰߳�ȫ��
		void Remove(EventService* ev);

		//���¼����������޸�һ���¼���������ע���¼�����,�����̰߳�ȫ��
		void Motify(EventService* ev);

		//timeʱ���ִ��timerCb����
		void RunAfter(Time time, std::function<void()> timerCb);

		//ÿ��timeʱ��ִ��һ��timerCb����
		void RunEvery(Time time, std::function<void()> timerCb);

		//����������̳߳����Դﵽ�첽ִ�е�Ч��
        void RunInThreadPool(std::function<void()>&& func);

	private:
		//��ǰmanager�������ţ���Щ������Ҫĳ��managerר�Ŵ���ĳ���¼�
		const int _idx;

		//�˳�ѭ���ı�־
		bool _quit;

		//���Ƴ����¼��б�Ҫ�Ƴ�ĳһ���¼����ȷ��ڸ��б��У�һ��ѭ�������Ż�������������
		std::vector<EventService*> _removedEv;

		//EventEpoller���ֵĻ�Ծ�¼����ŵ��б�
		std::vector<EventService*> _actEvServs;

		//��Ծ�¼��������ȼ����ŵ��б�
		std::vector<EventService*> _priorityEvQue[EVENT_PRIORITY_TYPE_COUNT];

		//��֤eventSet���̰߳�ȫ
		std::mutex _eventSetMutex;

		//��֤�Ƴ��¼�ʱ���̰߳�ȫ
		std::mutex _removedEvMutex;

		//�̳߳أ��ɽ��������������첽ִ��
		ThreadPool* _pThreadPool;

		//ѭ�������ڵ�����һ���߳���
		std::thread* _pLooper;

		//��ʱ�������ж�ʱ�¼�����
		Timer* _pTimer;

		//�¼�������
		EventEpoller _epoller;

		//�¼�����
		std::set<EventService*> _eventSet;

	};

}
