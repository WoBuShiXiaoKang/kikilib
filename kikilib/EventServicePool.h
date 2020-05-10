#pragma once
#include "Socket.h"

namespace kikilib
{
	class EventManager;
	class EventService;

	//�¼�����������
	//ְ�������¼�����ʵ������
	class EventServicePool
	{
	public:
		EventServicePool() {};
		virtual ~EventServicePool() {};

		//����EventService����ʵ���ĺ���
		virtual EventService* CreateEventService(Socket& sock, EventManager* evMgr) = 0;

		virtual void RetrieveEventService(EventService* ev) = 0;
	};

}