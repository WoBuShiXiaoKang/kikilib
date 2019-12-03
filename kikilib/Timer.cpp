#include "Timer.h"
#include "LogManager.h"

#include <sys/timerfd.h>
#include <string.h>

using namespace kikilib;

void Timer::RunExpired()
{
	Time nowTime = Time::now();
	bool isDo = true;
	std::map<Time, std::function<void()>>::iterator it, end;
	while(isDo)
	{//�����ô���ӣ�����Ϊit->second����������п��ܲ�����ʱ���ģ����ʱ��ֻ�������Ų�������
		{
			std::lock_guard<std::mutex> lock(_timerMutex);
			it = _timerCbMap.begin();
			end = _timerCbMap.end();
			if (it != end && it->first < nowTime)
			{
				isDo = true;
			}
		}
		
		if (isDo)
		{
			(it->second)();
		}
		std::lock_guard<std::mutex> lock(_timerMutex);
		_timerCbMap.erase(it);
	}
	
	if (!_timerCbMap.empty())
	{
		std::map<Time, std::function<void()>>::iterator it;

		{//������RunAt����ΪRunAt��Ҳ��ͬ�����������������
			std::lock_guard<std::mutex> lock(_timerMutex);
			it = _timerCbMap.begin();
		}
		
		RunAt(it->first, it->second);
	}
}

void Timer::RunAt(Time time, std::function<void()> cb)
{
	bool needSetTime = false;

	{
		std::lock_guard<std::mutex> lock(_timerMutex);
		_timerCbMap.insert(std::move(std::pair<Time, std::function<void()>>(time, cb)));
		if (_timerCbMap.begin()->first == time)
		{//�¼�������������������������Ҫ����timefd�����õ�ʱ��
			needSetTime = true;
		}
	}
	
	if (needSetTime)
	{
		struct itimerspec newValue;
		struct itimerspec oldValue;
		memset(&newValue, 0, sizeof newValue);
		memset(&oldValue, 0, sizeof oldValue);
		newValue.it_value = time.TimeIntervalFromNow();
		int ret = ::timerfd_settime(_timeSock.fd(), 0, &newValue, &oldValue);
		if (ret)
		{
			RecordLog(ERROR_DATA_INFORMATION, std::string("timerfd_settime failed. errno : ") + std::to_string(errno));
		}
	}
}
