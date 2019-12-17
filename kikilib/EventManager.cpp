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
	: _idx(idx), _quit(false), _pThreadPool(threadPool),  _pLooper(nullptr), _pTimer(nullptr), _pCtx(nullptr)
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

bool EventManager::loop()
{
	if (_pThreadPool == nullptr)
	{//�ж��̳߳ع����Ƿ���Ч
		RecordLog(ERROR_DATA_INFORMATION, std::to_string(_idx) + " get a null threadpool!");
		return false;
	}
	//��ʼ��EventEpoller
	if (!_epoller.init())
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
	EventService* pTimerServe = new TimerEventService(timeSock, this);
	//���ö�ʱ���¼�Ϊ���ȼ���ߵ��¼�
	pTimerServe->setEventPriority(IMMEDIATE_EVENT);

	insertEv(pTimerServe);

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
				this->_epoller.getActEvServ(Parameter::epollTimeOutMs, this->_actEvServs);
				//ÿ��epoll����һ�η���������ʱ��
				//Time::UpdataRoughTime();
				//�����ȼ��ֶ�
				for (auto pEvServ : this->_actEvServs)
				{
					if (pEvServ->getEventPriority() >= IMMEDIATE_EVENT)
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
						evServ->handleEvent();
					}
				}
				//�����ٹ�ע���¼�
				for (auto unusedEv : this->_removedEv)
				{
					//�Ӽ����¼����Ƴ�
					this->_epoller.removeEv(unusedEv);
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
void EventManager::insertEv(EventService* ev)
{
	if (!ev)
	{
		return;
	}

	if (ev->isConnected())
	{//insert��ζ�������¼��Ĵ���
		ev->handleConnectionEvent();
	}
	else
	{
		return;
	}
	
	if (ev->isConnected())
	{

		{
			std::lock_guard<std::mutex> lock(_eventSetMutex);
			_eventSet.insert(ev);
		}

		_epoller.addEv(ev);
	}
	
}

//���¼����������Ƴ�һ���¼�
void EventManager::removeEv(EventService* ev)
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
void EventManager::modifyEv(EventService* ev)
{
	if (!ev)
	{
		return;
	}
	bool isNewEv = false;

	{
		std::lock_guard<std::mutex> lock(_eventSetMutex);
		if (_eventSet.find(ev) == _eventSet.end())
		{
			isNewEv = true;
		}
	}

	if (isNewEv)
	{
		insertEv(ev);
	}
	else
	{
		_epoller.modifyEv(ev);
	}
}

void EventManager::runAt(Time time, std::function<void()>&& timerCb)
{
	std::lock_guard<std::mutex> lock(_timerMutex);
	_pTimer->runAt(time, std::move(timerCb));
}

void EventManager::runAt(Time time, std::function<void()>& timerCb)
{
	std::lock_guard<std::mutex> lock(_timerMutex);
	_pTimer->runAt(time, timerCb);
}

//timeʱ���ִ��timerCb����
void EventManager::runAfter(Time time, std::function<void()>&& timerCb)
{
	Time runTime(Time::now().getTimeVal() + time.getTimeVal());

	{
		std::lock_guard<std::mutex> lock(_timerMutex);
		_pTimer->runAt(runTime, std::move(timerCb));
	}
}

//timeʱ���ִ��timerCb����
void EventManager::runAfter(Time time, std::function<void()>& timerCb)
{
	Time runTime(Time::now().getTimeVal() + time.getTimeVal());

	{
		std::lock_guard<std::mutex> lock(_timerMutex);
		_pTimer->runAt(runTime, timerCb);
	}
}

//timeʱ���ִ��timerCb����
void EventManager::runEvery(Time time, std::function<void()> timerCb)
{
	//����lamda���ʽ�������Ż�����ִ��
	std::function<void()> realTimerCb(
		[this, time, timerCb]
		{
			timerCb();
			this->runEvery(time, timerCb);
		}
		);

	runAfter(time, std::move(realTimerCb));

}

//ÿ��timeʱ��ִ��һ��timerCb����,ֱ��isContinue��������false
void EventManager::runEveryUntil(Time time, std::function<void()> timerCb, std::function<bool()> isContinue)
{
	std::function<void()> realTimerCb(
		[this, time, timerCb, isContinue]
		{
			if (isContinue())
			{
				timerCb();
				this->runEveryUntil(time, timerCb, isContinue);
			}
		}
		);

	runAfter(time, std::move(realTimerCb));
}

//���������Ѿ���ʱ����Ҫִ�еĺ���
void EventManager::runExpired()
{
	std::lock_guard<std::mutex> lock(_timerQueMutex);

	{
		std::lock_guard<std::mutex> lock(_timerMutex);
		_pTimer->getExpiredTask(_actTimerTasks);
	}
	
	for (auto& task : _actTimerTasks)
	{
		task();
	}

	_actTimerTasks.clear();
}

//����������̳߳����Դﵽ�첽ִ�е�Ч��
void EventManager::runInThreadPool(std::function<void()>&& func)
{
    _pThreadPool->enqueue(std::move(func));
}

//����EventManager����Ψһ������������
void EventManager::setEvMgrCtx(void* ctx)
{
	std::lock_guard<std::mutex> lock(_ctxMutex);
	_pCtx = ctx;
}

//����EventManager����Ψһ������������
void* EventManager::getEvMgrCtx() 
{
	return _pCtx; 
}
