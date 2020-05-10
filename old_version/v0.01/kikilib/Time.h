//@Author Liu Yukang 
#pragma once

#include <stdint.h>

struct timespec;

namespace kikilib
{
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

		//�����ڵ�ʱ��
		struct timespec TimeIntervalFromNow();

		int64_t GetTimeVal() { return _timeVal; }

	private:
		int64_t _timeVal;

	};

	inline bool operator < (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() < rhs.GetTimeVal();
	}

	inline bool operator <= (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() <= rhs.GetTimeVal();
	}

	inline bool operator > (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() > rhs.GetTimeVal();
	}

	inline bool operator >= (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() >= rhs.GetTimeVal();
	}

	inline bool operator == (Time lhs, Time rhs)
	{
		return lhs.GetTimeVal() == rhs.GetTimeVal();
	}

}
