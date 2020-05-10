//@Author Liu Yukang 
#pragma once
#include "Socket.h"

namespace kikilib
{
	class EventManager;
	class EventService;

	//�¼�����������
	//ְ�������¼�����ʵ������
	template<class ConcreteEventService>
	class EventServiceFactory
	{
	public:
		EventServiceFactory() {};
		~EventServiceFactory() {};

		//�û������¼��Ĵ���EventService����ʵ���ĺ���
		EventService* CreateEventService(Socket sock, EventManager* evMgr)
		{
			return new ConcreteEventService(sock, evMgr);
		}
	};

}