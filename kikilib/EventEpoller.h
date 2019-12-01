#pragma once
#include "utils.h"

#include <vector>

struct epoll_event;

namespace kikilib
{
	class EventService;

	//�¼�������
	//ְ��
	//1�����ӷ��ֵ�ǰ���������¼�
	//2���޸ı����ӵ��¼�
	class EventEpoller
	{
	public:
		EventEpoller();
		~EventEpoller();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventEpoller);

		//�޸�EventEpoller�е��¼�
		void MotifyEv(EventService* evServ);

		//��EventEpoller������¼�
		void AddEv(EventService* evServ);

		//��EventEpoller���Ƴ��¼�
		void RemoveEv(EventService* evServ);

		//��ȡ��������¼�����
		void GetActEvServ(int timeOutMs, std::vector<EventService*>& activeEvServs);

	private:

		int _epollFd;
		std::vector<struct epoll_event> _activeEpollEvents;

	};

}
