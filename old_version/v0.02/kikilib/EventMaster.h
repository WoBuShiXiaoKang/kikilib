//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "EventServiceFactory.h"
#include "ManagerSelector.h"
#include "EventManager.h"

#include "ThreadPool.h"
#include "LogManager.h"

#include "utils.h"
#include "Parameter.h"

#include <fcntl.h>
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
		bool Init(int mgrCnt, int listenPort);

		//����EventManager����Ψһ������������
		//idxΪҪ���ü���EventManager������������
		//ctxΪҪ���õ�EventManager����������
		//������Դ���������³�����
		//ÿ��EventManager�����е��¼���Ҫ����һ��LRU��������ʱ������LRU����
		//��������ÿ��EventManager����ȫ�ֵģ���ô����Ҫ�������������ָ���ˡ�
		//EventManager���������ö����������Ĭ��Ϊnullptr
		bool SetEvMgrCtx(int idx, void* ctx);

		//ѭ��accept
		void Loop();

		void Stop() { _stop = true; }

	private:

		bool _stop;

		//std::thread* _acceptor;

		//�����¼�����Ĺ���
		EventServiceFactory<ConcreteEventService> _pEvServeFac;

		//�̳߳�
		ThreadPool* _pThreadPool;

		//���ڼ�����socket
		Socket _listener;

		//�¼���������ѡ����������ѡ����һ���¼����ĸ��¼�����������
		ManagerSelector _mgrSelector;

		//�¼��������б�
		std::vector<EventManager*> _evMgrs;
	};

	template<class ConcreteEventService>
	inline EventMaster<ConcreteEventService>::EventMaster()
		: _stop(true)
	{
		StartLogMgr(Parameter::logName);
		_pThreadPool = new ThreadPool();
	}

	template<class ConcreteEventService>
	inline EventMaster<ConcreteEventService>::~EventMaster()
	{
		Stop();
		if (_pThreadPool)
		{
			delete _pThreadPool;
		}
		EndLogMgr();
	}

	//�������EventManager�߳�
	//mgrCnt : �����¼���������������һ���¼���������Ӧһ���߳�
	//listenPort : Ҫ���ĸ��˿���ѭ��listen
	template<class ConcreteEventService>
	inline bool EventMaster<ConcreteEventService>::Init(int mgrCnt, int listenPort)
	{
		//��ʼ�������׽���
		if (_listener.IsUseful())
		{
			_listener.SetTcpNoDelay(Parameter::isNoDelay);
			_listener.SetReuseAddr(true);
			_listener.SetReusePort(true);
			_listener.SetBlockSocket();
			if (_listener.Bind(listenPort) < 0)
			{
				return false;
			}
			_listener.Listen();
		}
		else
		{
			RecordLog("listener unuseful!");
			return false;
		}

		//��ʼ�����ؾ�����
		_mgrSelector.SetManagerCnt(mgrCnt);

		//��ʼ��EventManager
		for (int i = 0; i < mgrCnt; ++i)
		{
			_evMgrs.emplace_back(std::move(new EventManager(i, _pThreadPool)));
			if (!_evMgrs.back()->Loop())
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
	inline bool EventMaster<ConcreteEventService>::SetEvMgrCtx(int idx, void* ctx)
	{
		if (idx >= _evMgrs.size() || idx < 0)
		{
			return false;
		}
		_evMgrs[idx]->SetEvMgrCtx(ctx);
		return true;
	}

	//ѭ��accept
	template<class ConcreteEventService>
	inline void EventMaster<ConcreteEventService>::Loop()
	{
		//һֱaccept����Ϊֻ��һ���߳���accept������û�о�Ⱥ����
		while (!_stop)
		{
			Socket conn(_listener.Accept());
			if (!conn.IsUseful() || conn.fd() > Parameter::maxEventServiceCnt)
			{
				continue;
			}

			conn.SetTcpNoDelay(Parameter::isNoDelay);
			RecordLog("accept a new usr ,ip : " + conn.GetIp());
			int nextMgrIdx = _mgrSelector.Next();
			EventService* ev = _pEvServeFac.CreateEventService(conn, _evMgrs[nextMgrIdx]);
			if (ev)
			{
				_evMgrs[nextMgrIdx]->Insert(ev);
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
