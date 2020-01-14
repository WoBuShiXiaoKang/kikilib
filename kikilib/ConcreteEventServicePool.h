//@Author Liu Yukang 
#pragma once
#include "EventServicePool.h"
#include "ObjPool.h"

namespace kikilib
{
	class EventManager;
	class EventService;

	//�¼�����������
	//ְ�������¼�����ʵ������
	template<class ConcreteEventService>
	class ConcreteEventServicePool : public EventServicePool
	{
	public:
		ConcreteEventServicePool() {};
		~ConcreteEventServicePool() {};

		//�û������¼��Ĵ���EventService����ʵ���ĺ���
		EventService* CreateEventService(Socket& sock, EventManager* evMgr)
		{
			return _evServeConstructor.New(sock, evMgr);
		}

		void RetrieveEventService(EventService* ev)
		{
			_evServeConstructor.Delete(static_cast<void*>(ev));
		}

	private:
		ObjPool<ConcreteEventService> _evServeConstructor;
	};

}