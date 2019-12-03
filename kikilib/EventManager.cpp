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

EventManager::EventManager(int idx) : _idx(idx), _quit(false), _pLooper(nullptr), _pTimer(nullptr)
{ }

EventManager::~EventManager()
{
	_quit = true;
	for (auto pEvServ : _eventSet)
	{
		delete pEvServ;
	}
	if (_pTimer)
	{
		delete _pTimer;
	}
	if (_pLooper)
	{
		_pLooper->join();
		delete _pLooper;
	}
}

void EventManager::Loop()
{
	//��ʼ����ʱ������
	int timeFd = ::timerfd_create(CLOCK_MONOTONIC,
		TFD_NONBLOCK | TFD_CLOEXEC);
	if (timeFd < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION,std::to_string(_idx) + " eventManager timer init failed!");
		return;
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
	//��ӳ�����ɾ���¼�
    std::lock_guard<std::mutex> lock(_eventSetMutex);
	auto it = _eventSet.find(ev);
	if (it != _eventSet.end())
	{
		_eventSet.erase(it);
	}
	//���뱻�Ƴ��¼��б�
	_removedEv.push_back(ev);
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
void EventManager::RunAfter(Time time, std::function<void()> timerCb)
{
	Time runTime(Time::now().GetTimeVal() + time.GetTimeVal());

	_pTimer->RunAt(runTime, timerCb);
}

//timeʱ���ִ��timerCb����
void EventManager::RunEvery(Time time, std::function<void()> timerCb)
{
	std::function<void()> realTimerCb(
		[this, time, timerCb]
		{
			timerCb();
			this->RunEvery(time, timerCb);
		}
		);

	RunAfter(time, realTimerCb);

}

//����������̳߳����Դﵽ�첽ִ�е�Ч��
template<class F, class... Args>
auto EventManager::RunInThreadPool(F&& f, Args&&... args)
->std::future<typename std::result_of<F(Args...)>::type>
{
    return _pThreadPool->enqueue(f,args...);
}
