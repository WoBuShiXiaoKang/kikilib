//@Author Liu Yukang 
#pragma once

#include <stdint.h>
#include <time.h>

struct timespec;

namespace kikilib
{
	//һ����ÿ���µ�������������
	const char days[12] = { 31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31 };

	//ʱ���࣬��λ��΢��us
	class Time
	{
	public:
		Time(int64_t msSinceEpoch) : _timeVal(msSinceEpoch) {}

		Time(const Time& time) { _timeVal = time._timeVal; }

		Time(const Time&& time) { _timeVal = time._timeVal; }

		Time& operator=(const Time& time)
		{
			_timeVal = time._timeVal;
			return *this;
		}

		~Time() {}

		//1970��1��1�յ����ڵ�΢����
		static Time now();

		//1970��1��1�յ����ڵ�����
		static time_t nowSec();

		//static void UpdataRoughTime();

		//static time_t GetRoughTime() { return _roughTime; }

		//���ݾ���1970-01-01 00:00:00������������������ʱ����ʱ����㵱ǰʱ����ʱ��
		static void toLocalTime(time_t second, long timezone, struct tm* tm_time);

		//�����ڵ�ʱ��
		struct timespec timeIntervalFromNow();

		int64_t getTimeVal() { return _timeVal; }

	private:
		int64_t _timeVal;

		//�ֲ�ʱ�䣬ÿ�ε���now��UpdataRoughTime����¸�ֵ������Ҫ��ȷʱ��ĳ������Ե���GetRoughTime��ȡ��ʱ�䡣��λΪ��s
		//static volatile time_t _roughTime;
	};

	inline bool operator < (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() < rhs.getTimeVal();
	}

	inline bool operator <= (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() <= rhs.getTimeVal();
	}

	inline bool operator > (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() > rhs.getTimeVal();
	}

	inline bool operator >= (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() >= rhs.getTimeVal();
	}

	inline bool operator == (Time lhs, Time rhs)
	{
		return lhs.getTimeVal() == rhs.getTimeVal();
	}

}
