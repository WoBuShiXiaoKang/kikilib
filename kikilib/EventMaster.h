//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "ManagerSelector.h"
#include "utils.h"
#include "EventMaster.h"
#include "EventService.h"
#include "LogManager.h"
#include "Parameter.h"
#include "EventManager.h"
#include "EventServiceFactory.h"
#include "ThreadPool.h"

#include <fcntl.h>
#include <vector>
//#include <thread>

namespace kikilib
{
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
		EventMaster()
			: _stop(false)
		{
			StartLogMgr(Parameter::logName);
			if (_listener.IsUseful())
			{
				_listener.SetTcpNoDelay(Parameter::isNoDelay);
				_listener.SetReuseAddr(true);
				_listener.SetReusePort(true);
				_listener.SetBlockSocket();
			}
			_storedFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
			_pThreadPool = new ThreadPool();
		}

		~EventMaster()
		{
			Stop();
			if (_pThreadPool)
			{
				delete _pThreadPool;
			}
			if (_storedFd >= 0)
			{
				::close(_storedFd);
			}
			EndLogMgr();
		}

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventMaster);

		//�������EventManager�߳�Ȼ��ѭ��accept
		//mgrCnt : �����¼���������������һ���¼���������Ӧһ���߳�
		//listenPort : Ҫ���ĸ��˿���ѭ��listen
		void Loop(int mgrCnt, int listenPort)
		{
			if (!_listener.IsUseful())
			{
				RecordLog("listener unuseful!");
				return;
			}

			if (_storedFd < 0)
			{
				RecordLog("_storedFd unuseful!");
				return;
			}

			if (_listener.Bind(listenPort) < 0)
			{
				return;
			}
			_listener.Listen();

			_mgrSelector.SetManagerCnt(mgrCnt);

			for (int i = 0; i < mgrCnt; ++i)
			{
				_evMgrs.emplace_back(std::move(new EventManager(i, _pThreadPool)));
				if (!_evMgrs.back()->Loop())
				{
					return;
					RecordLog("eventManager loop failed!");
				}
			}

			//һֱaccept����Ϊֻ��һ���߳���accept������û�о�Ⱥ����
			while (!_stop)
			{
				Socket conn(_listener.Accept());
				if (!conn.IsUseful())
				{
					//if (errno == EMFILE)
					//{
					//	::close(_storedFd);
					//	_storedFd = ::accept(_listener.fd(), NULL, NULL);
					//	::close(_storedFd);
					//	_storedFd = ::open("/dev/null", O_RDONLY | O_CLOEXEC);
					//}

					//if (_storedFd < 0)
					//{
					//	RecordLog("_storedFd unuseful after give a new connection!");
					//	break;
					//}
					continue;
				}
				conn.SetTcpNoDelay(Parameter::isNoDelay);
				RecordLog("accept a new usr ,ip : " + conn.GetIp());
				int nextMgrIdx = _mgrSelector.Next();
				EventService* ev = _pEvServeFac.CreateEventService(conn, _evMgrs[nextMgrIdx]);
				if (ev)
				{
					ev->HandleConnectionEvent();
					if (ev->IsConnected())
					{
						_evMgrs[nextMgrIdx]->Insert(ev);
					}
				}
			}

			//�ر�����evMgr�еķ���
			for (auto evMgr : _evMgrs)
			{
				delete evMgr;
			}
		}

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

		int _storedFd;
	};


}
