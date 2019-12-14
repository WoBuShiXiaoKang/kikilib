//@Author Liu Yukang 
#include "EventEpoller.h"
#include "LogManager.h"
#include "EventService.h"

#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <errno.h>

using namespace kikilib;

EventEpoller::EventEpoller()
	: _epollFd(-1), _activeEpollEvents(Parameter::epollEventListFirstSize)
{ }

EventEpoller::~EventEpoller() 
{
	if (_epollFd >= 0 )
	{
		::close(_epollFd);
	}
};

bool EventEpoller::Init()
{
	_epollFd = ::epoll_create1(EPOLL_CLOEXEC);
	return IsEpollFdUsefulAndMark();
}

bool EventEpoller::IsEpollFdUsefulAndMark()
{
	if (_epollFd < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("epoll fd unuseful. errno : "));
		return false;
	}
	return true;
}

//�޸�EventEpoller�е��¼�
void EventEpoller::MotifyEv(EventService* evServ)
{
	if (!IsEpollFdUsefulAndMark())
	{
		return;
	}
	if (!evServ)
	{
		RecordLog(WARNING_DATA_INFORMATION, "tring to motify a nullptr event!");
		return;
	}
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = evServ->GetInteresEv();
	event.data.ptr = evServ;
	int fd = evServ->GetFd();
	if (::epoll_ctl(_epollFd, EPOLL_CTL_MOD, fd, &event) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("event motify. errno : ") + std::to_string(errno));
	}
}

//��EventEpoller������¼�
void EventEpoller::AddEv(EventService* evServ)
{
	if (!IsEpollFdUsefulAndMark())
	{
		return;
	}
	if (!evServ)
	{
		RecordLog(WARNING_DATA_INFORMATION, "tring to all a nullptr event!");
		return;
	}
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = evServ->GetInteresEv();
	event.data.ptr = evServ;
	int fd = evServ->GetFd();
	if (::epoll_ctl(_epollFd, EPOLL_CTL_ADD, fd, &event) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("event add. errno : ") + std::to_string(errno));
	}
}

//��EventEpoller���Ƴ��¼�
void EventEpoller::RemoveEv(EventService* evServ)
{
	if (!IsEpollFdUsefulAndMark())
	{
		return;
	}
	if (!evServ)
	{
		return;
	}
	struct epoll_event event;
	memset(&event, 0, sizeof(event));
	event.events = evServ->GetInteresEv();
	event.data.ptr = evServ;
	int fd = evServ->GetFd();
	if (::epoll_ctl(_epollFd, EPOLL_CTL_DEL, fd, &event) < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, std::string("event remove. errno : ") + std::to_string(errno));
	}
}

void EventEpoller::GetActEvServ(int timeOutMs, std::vector<EventService*>& activeEvServs)
{
	if (!IsEpollFdUsefulAndMark())
	{
		return;
	}
	int actEvNum = ::epoll_wait(_epollFd, &*_activeEpollEvents.begin(), static_cast<int>(_activeEpollEvents.size()), timeOutMs);
	int savedErrno = errno;
	if (actEvNum > 0)
	{
		if (actEvNum > static_cast<int>(_activeEpollEvents.size()))
		{
			RecordLog(ERROR_DATA_INFORMATION, "unknown err in GetActServ()!");
			return;
		}
		for (int i = 0; i < actEvNum; ++i)
		{
			//�����¼����ͣ��Ž���Ծ�¼��б���
			EventService* evServ = static_cast<EventService*>(_activeEpollEvents[i].data.ptr);
			evServ->SetEventState(_activeEpollEvents[i].events);
			activeEvServs.push_back(evServ);
		}
		if (actEvNum == static_cast<int>(_activeEpollEvents.size()))
		{
			//����epoll�л�ȡ�¼����������ˣ�˵���������Ĵ�С���ܲ�������չһ��
			_activeEpollEvents.resize(_activeEpollEvents.size() * 2);
		}
	}
	else if (actEvNum == 0)
	{
		//RecordLog(DEBUG_DATA_INFORMATION, "nothing happened in GetActEvServ()");
	}
	else
	{
		if (savedErrno != EINTR)
		{
			RecordLog(ERROR_DATA_INFORMATION, std::string("epoll_wait. errno : ") + std::to_string(savedErrno));
		}
	}
}
