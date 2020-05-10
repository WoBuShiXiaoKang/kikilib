//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "ConcreteEventServicePool.h"
#include "ManagerSelector.h"
#include "EventManager.h"

#include "ThreadPool.h"
#include "LogManager.h"

#include "utils.h"
#include "Parameter.h"

//#include <fcntl.h>
#include <vector>
//#include <thread>

namespace kikilib
{
	class EventService;

	//�¼�������
	//ְ��
	//1�������������е��¼�������
	//2�������µ��������ӣ������µ��¼�����ʵ�壬Ȼ�����ManagerSelector��ѡ�����ѡ���������¼��Ĺ�����
	//3�������̳߳ع���ʵ��
	//��Ҫ�����������������
	//1���̳߳ع���threadpool
	//2���¼�������eventmanager
	//3���¼����񹤳�eventservicefactory
	template<class ConcreteEventService>
	class EventMaster
	{
	public:
		EventMaster();

		~EventMaster();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventMaster);

		//�������EventManager�߳�
		//mgrCnt : �����¼���������������һ���¼���������Ӧһ���߳�
		//listenPort : Ҫ���ĸ��˿���ѭ��listen
		bool init(int mgrCnt, int listenPort);

		//����EventManager����Ψһ������������
		//idxΪҪ���ü���EventManager������������
		//ctxΪҪ���õ�EventManager����������
		//������Դ���������³�����
		//ÿ��EventManager�����е��¼���Ҫ����һ��LRU��������ʱ������LRU����
		//��������ÿ��EventManager����ȫ�ֵģ���ô����Ҫ�������������ָ���ˡ�
		//EventManager���������ö����������Ĭ��Ϊnullptr
		bool setEvMgrCtx(int idx, void* ctx);

		//ѭ��accept
		void loop();

		void stop() { _stop = true; }

	private:

		bool _stop;

		//std::thread* _acceptor;

		//�̳߳�
		ThreadPool* _pThreadPool;

		//���ڼ�����socket
		Socket _listener;

		//�¼���������ѡ����������ѡ����һ���¼����ĸ��¼�����������
		ManagerSelector* _pMgrSelector;

		//�¼��������б�
		std::vector<EventManager*> _evMgrs;
	};

	template<class ConcreteEventService>
	inline EventMaster<ConcreteEventService>::EventMaster()
		: _stop(true), _pThreadPool(nullptr), _pMgrSelector(nullptr)
	{
		StartLogMgr(Parameter::logName);
		_pThreadPool = new ThreadPool();
		_pMgrSelector = new ManagerSelector(_evMgrs);
	}

	template<class ConcreteEventService>
	inline EventMaster<ConcreteEventService>::~EventMaster()
	{
		stop();
		if (_pThreadPool)
		{
			delete _pThreadPool;
		}
		if (_pMgrSelector)
		{
			delete _pMgrSelector;
		}
		EndLogMgr();
	}

	//�������EventManager�߳�
	//mgrCnt : �����¼���������������һ���¼���������Ӧһ���߳�
	//listenPort : Ҫ���ĸ��˿���ѭ��listen
	template<class ConcreteEventService>
	inline bool EventMaster<ConcreteEventService>::init(int mgrCnt, int listenPort)
	{
		//��ʼ�������׽���
		if (_listener.isUseful())
		{
			_listener.setTcpNoDelay(Parameter::isNoDelay);
			_listener.setReuseAddr(true);
			_listener.setReusePort(true);
			_listener.setBlockSocket();
			if (_listener.bind(listenPort) < 0)
			{
				return false;
			}
			_listener.listen();
		}
		else
		{
			RecordLog("listener unuseful!");
			return false;
		}

		//��ʼ��EventManager
		for (int i = 0; i < mgrCnt; ++i)
		{
			EventServicePool* pool = new ConcreteEventServicePool<ConcreteEventService>();
			_evMgrs.emplace_back(std::move(new EventManager(i, pool, _pThreadPool)));
			if (!_evMgrs.back()->loop())
			{
				return false;
				RecordLog("eventManager loop failed!");
			}
		}

		_stop = false;
		return true;
	}

	//����EventManager����Ψһ������������
	//idxΪҪ���ü���EventManager������������
	//ctxΪҪ���õ�EventManager����������
	template<class ConcreteEventService>
	inline bool EventMaster<ConcreteEventService>::setEvMgrCtx(int idx, void* ctx)
	{
		if (idx >= _evMgrs.size() || idx < 0)
		{
			return false;
		}
		_evMgrs[idx]->setEvMgrCtx(ctx);
		return true;
	}

	//ѭ��accept
	template<class ConcreteEventService>
	inline void EventMaster<ConcreteEventService>::loop()
	{
		//һֱaccept����Ϊֻ��һ���߳���accept������û�о�Ⱥ����
		while (!_stop)
		{
			Socket conn(_listener.accept());
			if (!conn.isUseful() || conn.fd() > Parameter::maxEventServiceCnt)
			{
				continue;
			}

			conn.setTcpNoDelay(Parameter::isNoDelay);
			RecordLog("accept a new usr ,ip : " + conn.ip());
			EventManager* pEvMgr = _pMgrSelector->next();
			EventService* ev = pEvMgr->CreateEventService(conn);
			if (ev)
			{
				pEvMgr->insertEv(ev);
			}
			else
			{
				RecordLog("create an eventservice failed!");
			}
		}

		//�ر�����evMgr�еķ���
		for (auto evMgr : _evMgrs)
		{
			delete evMgr;
		}
	}
}
