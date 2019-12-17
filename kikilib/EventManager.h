//@Author Liu Yukang 
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
		bool loop();

		//���¼��������в���һ���¼�,�����̰߳�ȫ��
		void insertEv(EventService* ev);

		//���¼����������Ƴ�һ���¼�,�����̰߳�ȫ��
		void removeEv(EventService* ev);

		//���¼����������޸�һ���¼���������ע���¼�����,�����̰߳�ȫ��
		void modifyEv(EventService* ev);

		//��Ҫע�⣬���timerCb�����ִ��RunExpired()�����Ļ��ᷢ������
		//��timeʱ��ִ��timerCb����
		//һ��time�Ͱ˸��ֽڣ��������൱��һ��ָ�뻹�Ƿ����˰˸��ֽڣ�x64��������time����&��&&
		void runAt(Time time, std::function<void()>&& timerCb);
		void runAt(Time time, std::function<void()>& timerCb);

		//timeʱ���ִ��timerCb����
		void runAfter(Time time, std::function<void()>&& timerCb);
		void runAfter(Time time, std::function<void()>& timerCb);

		//ÿ��timeʱ��ִ��һ��timerCb����
		void runEvery(Time time, std::function<void()> timerCb);

		//ÿ��timeʱ��ִ��һ��timerCb����,ֱ��isContinue��������false
		void runEveryUntil(Time time, std::function<void()> timerCb, std::function<bool()> isContinue);

		//���������Ѿ���ʱ����Ҫִ�еĺ���
		void runExpired();

		//�������������̳߳����Դﵽ�첽ִ�е�Ч��
        void runInThreadPool(std::function<void()>&& func);

		//����EventManager����Ψһ������������
		//�������³�����
		//ÿ��EventManager�����е��¼���Ҫ����һ��LRU��������ʱ������LRU����
		//��������ÿ��EventManager����ȫ�ֵģ���ô����Ҫ�������������ָ���ˡ�
		//EventManager���������ö����������Ĭ��Ϊnullptr
		void setEvMgrCtx(void* ctx);

		//���EventManager����Ψһ������������
		void* getEvMgrCtx();

	private:
		//��ǰmanager�������ţ���Щ������Ҫĳ��managerר�Ŵ���ĳ���¼�
		const int _idx;

		//�˳�ѭ���ı�־
		bool _quit;

		//�̳߳أ��ɽ��������������첽ִ��
		ThreadPool* _pThreadPool;

		//ѭ�������ڵ�����һ���߳���
		std::thread* _pLooper;

		//��ʱ�������ж�ʱ�¼�����
		Timer* _pTimer;

		//��֤eventSet���̰߳�ȫ
		std::mutex _eventSetMutex;

		//��֤timer�̰߳�ȫ
		std::mutex _timerMutex;

		//��֤_actTimerTasksʹ�õ��̰߳�ȫ
		std::mutex _timerQueMutex;

		//��֤_removedEv�¼��б���̰߳�ȫ
		std::mutex _removedEvMutex;

		//��֤����������ָ����̰߳�ȫ
		std::mutex _ctxMutex;

		//���Ƴ����¼��б�Ҫ�Ƴ�ĳһ���¼����ȷ��ڸ��б��У�һ��ѭ�������Ż�����delete
		std::vector<EventService*> _removedEv;

		//EventEpoller���ֵĻ�Ծ�¼����ŵ��б�
		std::vector<EventService*> _actEvServs;

		//Timer���ֵĳ�ʱ�¼����ŵ��б�
		std::vector<std::function<void()>> _actTimerTasks;

		//��Ծ�¼��������ȼ����ŵ��б�
		std::vector<EventService*> _priorityEvQue[EVENT_PRIORITY_TYPE_COUNT];

		//�¼�������
		EventEpoller _epoller;

		//�¼�����
		std::set<EventService*> _eventSet;

		//EventManager����Ψһ������������
		//�������³�����
		//ÿ��EventManager�����е��¼���Ҫ����һ��LRU��������ʱ������LRU����
		//��������ÿ��EventManager����ȫ�ֵģ���ô����Ҫ�������������ָ���ˡ�
		//EventManager���������ö����������Ĭ��Ϊnullptr
		void* _pCtx;
	};

}
