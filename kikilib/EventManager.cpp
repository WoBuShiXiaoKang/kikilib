//@Author Liu Yukang 
#include "EventManager.h"
#include "LogManager.h"
#include "EventService.h"
#include "TimerEventService.h"
#include "Timer.h"
#include "ThreadPool.h"

#include <sys/timerfd.h>
#include <sys/epoll.h>
#include <signal.h>

using namespace kikilib;

EventManager::EventManager(int idx, ThreadPool* threadPool) 
	: _idx(idx), _quit(false), _pThreadPool(threadPool),  _pLooper(nullptr), _pTimer(nullptr)
{ }

EventManager::~EventManager()
{
	_quit = true;
	RecordLog(ERROR_DATA_INFORMATION, std::to_string(_idx) + " EventManager being deleted!");
	if (_pLooper)
	{
		_pLooper->join();
		delete _pLooper;
	}
	for (auto pEvServ : _eventSet)
	{
		delete pEvServ;
	}
	if (_pTimer)
	{
		delete _pTimer;
	}
}

bool EventManager::Loop()
{
	if (_pThreadPool == nullptr)
	{//�ж��̳߳ع����Ƿ���Ч
		RecordLog(ERROR_DATA_INFORMATION, std::to_string(_idx) + " get a null threadpool!");
		return false;
	}
	//��ʼ��EventEpoller
	if (!_epoller.Init())
	{
		RecordLog(ERROR_DATA_INFORMATION, std::to_string(_idx) + " init epoll fd failed!");
		return false;
	}
	//��ʼ����ʱ������
	int timeFd = ::timerfd_create(CLOCK_MONOTONIC,
		TFD_NONBLOCK | TFD_CLOEXEC);
	if (timeFd < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION,std::to_string(_idx) + " eventManager timer init failed!");
		return false;
	}
	Socket timeSock(timeFd);
	_pTimer = new Timer(timeSock);
	EventService* pTimerServe = new TimerEventService(_pTimer, timeSock, this);
	//���ö�ʱ���¼�Ϊ���ȼ���ߵ��¼�
	pTimerServe->SetEventPriority(IMMEDIATE_EVENT);

	Insert(pTimerServe);

	//��ʼ��loop
	_pLooper = new std::thread(
		[this]
		{
			while (!this->_quit)
			{
				//��������б�
				if(this->_actEvServs.size())
                {
                    this->_actEvServs.clear();
                }
				for (int i = 0; i < EVENT_PRIORITY_TYPE_COUNT; ++i)
				{
				    if(this->_priorityEvQue[i].size())
                    {
                        this->_priorityEvQue[i].clear();
                    }
				}
				//��ȡ��Ծ�¼�
				this->_epoller.GetActEvServ(Parameter::epollTimeOutMs, this->_actEvServs);
				//�����ȼ��ֶ�
				for (auto pEvServ : this->_actEvServs)
				{
					if (pEvServ->GetEventPriority() >= IMMEDIATE_EVENT)
					{
						(this->_priorityEvQue[IMMEDIATE_EVENT]).push_back(pEvServ);
					}
					else
					{
						(this->_priorityEvQue[NORMAL_EVENT]).push_back(pEvServ);
					}
				}
				//�������ȼ������¼�
				for (int i = 0; i < EVENT_PRIORITY_TYPE_COUNT; ++i)
				{
					for (auto evServ : this->_priorityEvQue[i])
					{
						evServ->HandleEvent();
					}
				}
				//�����ٹ�ע���¼�
				for (auto unusedEv : this->_removedEv)
				{
					//�Ӽ����¼����Ƴ�
					this->_epoller.RemoveEv(unusedEv);
					//close���fd
					delete unusedEv;
				}
				if(this->_removedEv.size())
                {
                    std::lock_guard<std::mutex> lock(_removedEvMutex);
                    this->_removedEv.clear();
                }
			}
		}
		);
	return true;
}

//���¼��������в���һ���¼�
void EventManager::Insert(EventService* ev)
{
	if (!ev)
	{
		return;
	}
	{
		std::lock_guard<std::mutex> lock(_eventSetMutex);
		_eventSet.insert(ev);
	}
	_epoller.AddEv(ev);
}

//���¼����������Ƴ�һ���¼�
void EventManager::Remove(EventService* ev)
{
	if (!ev)
	{
		return;
	}
	
	{//��ӳ�����ɾ���¼�
		std::lock_guard<std::mutex> lock(_eventSetMutex);
		auto it = _eventSet.find(ev);
		if (it != _eventSet.end())
		{
			_eventSet.erase(it);
		}
	}
	
	{//���뱻�Ƴ��¼��б�
		std::lock_guard<std::mutex> lock(_removedEvMutex);
		_removedEv.push_back(ev);
	}
}

//���¼����������޸�һ���¼���������ע���¼�����
void EventManager::Motify(EventService* ev)
{
	if (!ev)
	{
		return;
	}

	{
		std::lock_guard<std::mutex> lock(_eventSetMutex);
		if (_eventSet.find(ev) == _eventSet.end())
		{
			Insert(ev);
			return;
		}
	}
	_epoller.MotifyEv(ev);
}

//timeʱ���ִ��timerCb����
void EventManager::RunAfter(Time time, std::function<void()>&& timerCb)
{
	Time runTime(Time::now().GetTimeVal() + time.GetTimeVal());

	_pTimer->RunAt(runTime, std::move(timerCb));
}

//timeʱ���ִ��timerCb����
void EventManager::RunEvery(Time time, std::function<void()> timerCb)
{
	//����lamda���ʽ�������Ż�����ִ�У�������
	std::function<void()> realTimerCb(
		[this, time, timerCb]
		{
			timerCb();
			this->RunEvery(time, timerCb);
		}
		);

	RunAfter(time, std::move(realTimerCb));

}

//ÿ��timeʱ��ִ��һ��timerCb����,ֱ��isContinue��������false
void EventManager::RunEveryUntil(Time time, std::function<void()> timerCb, std::function<bool()> isContinue)
{
	std::function<void()> realTimerCb(
		[this, time, timerCb, isContinue]
		{
			if (isContinue())
			{
				timerCb();
				this->RunEveryUntil(time, timerCb, isContinue);
			}
		}
		);

	RunAfter(time, std::move(realTimerCb));
}

//����������̳߳����Դﵽ�첽ִ�е�Ч��
void EventManager::RunInThreadPool(std::function<void()>&& func)
{
    return _pThreadPool->enqueue(std::move(func));
}
