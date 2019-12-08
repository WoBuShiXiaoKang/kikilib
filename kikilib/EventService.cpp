//@Author Liu Yukang 
#include "EventService.h"
#include "EventManager.h"
#include "LogManager.h"

#include <poll.h>
#include <sys/epoll.h>

using namespace kikilib;

EventService::EventService(Socket& sock, EventManager* evMgr, int interestEvent)
	: _interestEvent(interestEvent), _eventState(0), _eventPriority(NORMAL_EVENT),
	_isConnected(true), _sock(sock), _pMyEvMgr(evMgr), _bufWritter(sock, this), _bufReader(sock)
{}

EventService::EventService(Socket&& sock, EventManager* evMgr, int interestEvent)
	: _interestEvent(interestEvent), _eventState(0), _eventPriority(NORMAL_EVENT),
	_isConnected(true), _sock(sock), _pMyEvMgr(evMgr), _bufWritter(sock, this), _bufReader(sock)
{}

void EventService::Close()
{
    if(_isConnected)
    {
        _isConnected = false;
        HandleCloseEvent();
        _pMyEvMgr->Remove(this);
    }

}

void EventService::SetInteresEv(int newInterestEv)
{
	_interestEvent = newInterestEv;
	_pMyEvMgr->Motify(this);
}

//�����¼����ʹ����¼�
void EventService::HandleEvent()
{
	if ((_eventState & EPOLLHUP) && !(_eventState & EPOLLIN))
	{
		Close();
	}

	if (_eventState & (EPOLLERR))
	{
        RecordLog(ERROR_DATA_INFORMATION, "error event in HandleEvent()!");
        HandleErrEvent();
        Close();
	}
	if (_eventState & (EPOLLIN | EPOLLPRI | EPOLLRDHUP))
	{
		if (!_bufReader.IsEmptyAfterRead())
		{
			HandleReadEvent();
		}
		else
		{
			Close();
		}
		/*else
		{
			RecordLog(ERROR_DATA_INFORMATION, "read fd error!");
			HandleErrEvent();
		}*/
	}
	if (_eventState & EPOLLOUT)
	{
		HandleWriteEvent();
	}
}

void EventService::HandleWriteEvent()
{
	_bufWritter.WriteBufToSock();
}

//дһ��int
void EventService::WriteInt32(int num)
{
	_bufWritter.SendInt32(num);
}

void EventService::WriteBuf(std::string& content)
{
	_bufWritter.Send(content);
}

void EventService::WriteBuf(std::string&& content)
{
	_bufWritter.Send(std::move(content));
}

//��ȡһ��int����������û�У��򷵻�false
bool EventService::ReadInt32(int& res)
{
	return _bufReader.ReadInt32(res);
}

std::string EventService::ReadBuf(size_t len)
{
	return _bufReader.Read(len);
}

std::string EventService::ReadAll()
{
	return _bufReader.ReadAll();
}

//��һ�У�������\r\n��β,��û�У����ؿմ�
std::string EventService::ReadLineEndOfRN()
{
	return _bufReader.ReadLineEndOfRN();
}

//��һ�У�������\r��β,��û�У����ؿմ�
std::string EventService::ReadLineEndOfR()
{
	return _bufReader.ReadLineEndOfR();
}

//��һ�У�������\n��β,��û�У����ؿմ�
std::string EventService::ReadLineEndOfN()
{
	return _bufReader.ReadLineEndOfN();
}

//timeʱ���ִ��timerCb����
void EventService::RunAfter(Time time, std::function<void()>&& timerCb)
{ 
	_pMyEvMgr->RunAfter(time, std::move(timerCb));
}

//ÿ��timeʱ��ִ��timerCb����
void EventService::RunEvery(Time time, std::function<void()> timerCb) 
{ 
	_pMyEvMgr->RunEvery(time, timerCb); 
}

//ÿ��timeʱ��ִ��һ��timerCb����,ֱ��isContinue��������false
void EventService::RunEveryUntil(Time time, std::function<void()> timerCb, std::function<bool()> isContinue)
{
	_pMyEvMgr->RunEveryUntil(time, timerCb, isContinue);
}

//����������̳߳����Դﵽ�첽ִ�е�Ч��
void EventService::RunInThreadPool(std::function<void()>&& func)
{
    return _pMyEvMgr->RunInThreadPool(std::move(func));
}
