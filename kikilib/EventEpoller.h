//@Author Liu Yukang 
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
	//ע�������epoll�õ�LTģʽ��ԭ�����£�
	//	  read�¼�����ʱ����server��ҵ�񲢲���ÿ��readall�����м�ʱ������ô���������client
	//����;޴���壬ETģʽ����ÿ�ν����ݶ����ڴ棬��server����ʱ����ͻᵼ�����ݶѻ����ڴ�
	//������ʹ��LT�������������⣬socket���ջ����������ĶԷ��ᷢ��ʧ�ܡ�
	class EventEpoller
	{
	public:
		EventEpoller();
		~EventEpoller();

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventEpoller);

		//Ҫʹ��EventEpoller������øú�����ʼ����ʧ���򷵻�false
		bool Init();

		//�޸�EventEpoller�е��¼�
		void MotifyEv(EventService* evServ);

		//��EventEpoller������¼�
		void AddEv(EventService* evServ);

		//��EventEpoller���Ƴ��¼�
		void RemoveEv(EventService* evServ);

		//��ȡ��������¼�����
		void GetActEvServ(int timeOutMs, std::vector<EventService*>& activeEvServs);

	private:

		bool IsEpollFdUsefulAndMark();

		int _epollFd;
		std::vector<struct epoll_event> _activeEpollEvents;

	};

}
