//@Author Liu Yukang 
#pragma once
#include "EventService.h"
#include "Time.h"
#include "Timer.h"
#include "utils.h"

namespace kikilib
{
	//��ʱ���¼�����
	//��ʱ�¼��ɶ�ʱ��ִ�д˿�������Ҫִ�еĺ���
	class TimerEventService : public EventService
	{
	public:
		TimerEventService(Timer* timer, Socket sock, EventManager* evMgr);

		~TimerEventService() {}

		DISALLOW_COPY_MOVE_AND_ASSIGN(TimerEventService);

		void HandleReadEvent();

	private:
		Timer* _pTimer;

	};

}
