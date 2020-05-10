//@Author Liu Yukang 
#include "LogManager.h"

using namespace kikilib;

//static��Ա������Ҫ�������ʼ��
LogManager* LogManager::_logMgr = nullptr;
std::mutex LogManager::_logMutex;
bool LogManager::_isInit = false;

LogManager::LogManager() :
	_curLogFileByte(0),
	_curLogFileIdx(0),
	_logFile(nullptr),
	_logLoop(nullptr),
	_stop(0L),
	_lastRead(-1L),
	_lastWrote(-1L),
	_writableSeq(0L),
	_logTimeStr("1970-01-01 00:00:00"),
	_logTimeSec(0),
	_timeZone(0)
{ }

bool LogManager::StartLogManager(std::string logPath)
{
	if (!_isInit)
	{
		std::lock_guard<std::mutex> lock(_logMutex);
		if (!_isInit)
		{
			_logPath = logPath;
			_logFile = ::fopen((_logPath + std::to_string(_curLogFileIdx & 1)).c_str(), "w");
			if (!_logFile)
			{
				printf("logfile open failed");
				return false;
			}
			std::string fileMsg = "cur file idx : 0\n";
			fwrite(fileMsg.c_str(), fileMsg.size(), 1, _logFile);
			fflush(_logFile);
			_isInit = true;
			//����һ���̳߳���д��־
			_logLoop = new std::thread(&LogManager::WriteDownLog, this);
		}
	}
	time_t tmpTime = 0;
	struct tm* tm = localtime(&tmpTime);
	_timeZone = 0 - tm->tm_hour * 60 * 60;//* 60 * 60�� 
	UpdateLogTime();
	return true;
}

void LogManager::EndLogManager()
{
	_stop.store(1L);
	_condition.notify_one();
	_logLoop->join();
	::fclose(_logFile);
	_isInit = false;
	delete _logLoop;
}

LogManager* LogManager::GetLogMgr()
{
	if (!_logMgr)
	{
		std::lock_guard<std::mutex> lock(_logMutex);
		if (!_logMgr)
		{
			_logMgr = new LogManager();
		}
	}
	//else if (!_isInit)
	//{
	//	return nullptr;
	//}
	return _logMgr;
}

void LogManager::Record(std::string& logDate)
{
	Record(std::move(logDate));
}

void LogManager::Record(const char* logData)
{
	Record(std::move(std::string(logData)));
}

void LogManager::Record(unsigned dataType, const char* logData)
{
	Record(std::move(std::string(LogDataTypeStr[dataType] + logData)));
}

void LogManager::Record(unsigned dataType, std::string& logData)
{
	Record(std::move(LogDataTypeStr[dataType] + logData));
}

void LogManager::Record(unsigned dataType, std::string&& logData)
{
	Record(std::move(LogDataTypeStr[dataType] + logData));
}

void LogManager::Record(std::string&& logDate)
{
	if (_writableSeq.load() - _lastRead.load() >= Parameter::kLogBufferLen - 1)
	{//���˷���������־����Ϊ�������������ǰ�����־���м�ֵ
		return;
	}
	const int64_t writableSeq = _writableSeq.fetch_add(1);
	
	//д����
	if (Parameter::isAddTimestampInLog)
	{
		if (Time::nowSec() > _logTimeSec)
		{//��ǰ�������Ĵ���ʱ�����־ϵͳ�洢��ʱ��Ҫ����1sʱ������־ϵͳ��ʱ���ַ���
			UpdateLogTime();
		}
		std::string cpLogTimeStr(_logTimeStr.c_str());
		std::string realLogData(cpLogTimeStr.size() + logDate.size() + 2, ' ');//+2����Ϊ��ʱ�����־�м���һ���ո�����һ�����з�
		//����ʱ��
		memcpy(&(*realLogData.begin()), &(*cpLogTimeStr.begin()), cpLogTimeStr.size());
		//������־��Ϣ
		memcpy(&(*(realLogData.begin() + cpLogTimeStr.size() + 1)), &(*logDate.begin()), logDate.size());
		//���ϻ��з�
		realLogData.back() = '\n';
		_ringBuf[writableSeq & (Parameter::kLogBufferLen - 1)] = std::move(realLogData);
	}
	else
	{
		logDate += '\n';
		_ringBuf[writableSeq & (Parameter::kLogBufferLen - 1)] = std::move(logDate);
	}
	
	//volatile int a = 0;
	while (writableSeq - 1L != _lastWrote.load())
	{//��Ϊд�����ٶ��൱����������һ�㲻������,��Ҫע��ģ����_lastWrote�����ֵ��volatile�ģ�������������д��û������
	}
	_lastWrote.store(writableSeq);

	if (writableSeq == _lastRead.load() + 1)
	{
		std::unique_lock<std::mutex> lock(_logMutex);
		_condition.notify_one();
	}
}

void LogManager::UpdateLogTime()
{
	std::lock_guard<std::mutex> lock(_timeMutex);
	if (Time::nowSec() > _logTimeSec)
	{//��ǰ�������Ĵ���ʱ�����־ϵͳ�洢��ʱ��Ҫ����1sʱ������־ϵͳ��ʱ���ַ���
		_logTimeSec = Time::nowSec();
		struct tm t;
		Time::ToLocalTime(_logTimeSec, _timeZone, &t);
		std::string cp(_logTimeStr.c_str());
		strftime(&(*cp.begin()), cp.size() + 1, "%Y-%m-%d %H:%M:%S", &t);
		_logTimeStr = std::move(cp);
	}
}

void LogManager::WriteDownLog()
{
	const int64_t kMaxPerLogFileByte = Parameter::maxLogDiskByte / 2;
	while (true)
	{
		if (_stop.load() && _lastRead.load() == _lastWrote.load())
		{
			return;
		}
		else
		{//ֻҪ��д���ͱ���Ҫд��
			//��ʼ��ӡ��ǰ��־���ݶ���
			while (_lastRead.load() < _lastWrote.load())
			{
				while (_lastRead.load() < _lastWrote.load())
				{
					int64_t curRead = _lastRead.load() + 1;
					if (_curLogFileByte > kMaxPerLogFileByte)
					{//����Ҫ�����ˣ������ɵ���־
						fflush(_logFile);
						::fclose(_logFile);
						++_curLogFileIdx;
						_logFile = ::fopen((_logPath + std::to_string(_curLogFileIdx & 1)).c_str(), "w");
						_curLogFileByte = 0;
						//���ȼ�¼��ǰ��־Ϊ�ڼ�����־��
						std::string fileMsg = "cur file idx : " + std::to_string(_curLogFileIdx) + '\n';
						fwrite(fileMsg.c_str(), fileMsg.size(), 1, _logFile);
					}
					std::string& buf = _ringBuf[curRead & (Parameter::kLogBufferLen - 1)];
					fwrite(buf.c_str(), buf.size(), 1, _logFile);
					_curLogFileByte += buf.size();
					buf.clear();
					_lastRead.store(curRead);
				}
				fflush(_logFile);
			}

			{//���û�����ˣ�������
				std::unique_lock<std::mutex> lock(_logMutex);
				if (!_stop.load() && _lastRead.load() == _lastWrote.load())
				{
					_condition.wait(lock);
				}
			}
		}
	}
}