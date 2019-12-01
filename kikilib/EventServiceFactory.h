#pragma once
#include "Socket.h"

namespace kikilib
{
	class EventManager;
	class EventService;

	//�¼�����������
	//ְ�������¼�����ʵ������
	//�÷���
	//�û��̳�EventServiceFactory�࣬ʵ��һ������EventService����ķ���
	//   Ȼ�󽫹�����ʵ������EventMaster��������������ת��ÿ��һ��
	//   �µ����ӵ�����EventMaster�ͻ�ʹ�ù�������һ�����¼��������Ϊ
	//   �µ����ӷ���
	class EventServiceFactory
	{
	public:
		EventServiceFactory() {};
		virtual ~EventServiceFactory() {};

		//�û������¼��Ĵ���EventService����ʵ���ĺ���
		virtual EventService* CreateEventService(Socket sock, EventManager* evMgr) = 0;
	};

}