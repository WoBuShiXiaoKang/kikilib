//@Author Liu Yukang 
#pragma once
#include <vector>

namespace kikilib
{
	class EventManager;

	enum scheduleStrategy
	{
		MIN_EVENT_FIRST = 0 , //�����¼�����
		ROUND_ROBIN			  //�����ַ�
	};

	//�¼�������ѡ������������һ���¼�Ӧ�÷����ĸ��¼���������
	class ManagerSelector
	{
	public:
		ManagerSelector(std::vector<EventManager*>& evMgrs, int strategy = MIN_EVENT_FIRST) :  _curMgr(-1) , _strategy(strategy) , _evMgrs(evMgrs) {}
		~ManagerSelector() {}

		//���÷ַ�����Ĳ���
		//MIN_EVENT_FIRST��ÿ����ѡEventService���ٵ�EventManager����������
		//ROUND_ROBIN��ÿ��������ѡEventManager����������
		void setStrategy(int strategy);

		EventManager* next();

	private:
		int _curMgr;

		int _strategy;

		std::vector<EventManager*>& _evMgrs;

	};

}