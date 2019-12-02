#include "LogManager.h"
#include "Parameter.h"

#define MIN_WRITABLE_LOGDATE_NUM 1 //������������С�������ʱ��д��־���̵߳ȴ���ô��WRITING_LOG_WAIT_TIME����
#define WRITING_LOG_WAIT_TIME 100 //������������С��MIN_WRITABLE_LOGDATE_NUMʱ��д��־���̵߳ȴ���ô��ms
#define BYTE_PER_LOGQUE_STRING 60 //��־����ÿ��stringԪ�ص��ֽڴ�С����ֵ

using namespace kikilib;

//static��Ա������Ҫ�������ʼ��
LogManager* LogManager::_logMgr = nullptr;
std::mutex LogManager::_logMutex;
bool LogManager::_isInit = false;

bool LogManager::StartLogManager(std::string logPath)
{
	if (!_isInit)
	{
		std::lock_guard<std::mutex> lock(_logMutex);
		if (!_isInit)
		{
			_logPath = logPath;
			_logFile = ::fopen((_logPath + std::to_string(_logFileIdx)).c_str(), "w");
			if (!_logFile)
			{
				return false;
			}
			_isInit = true;
			//����һ���̳߳���д��־
			_logLoop = new std::thread(&LogManager::WriteDownLog, this);
		}
	}
	return true;
}

void LogManager::EndLogManager()
{
	_stop = true;
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
	std::lock_guard<std::mutex> lock(_logMutex);//�̰߳�ȫ
	int curQue = _recordableQue.load();
	_logQue[curQue].emplace(logDate);
	if (_logQue[curQue].size() == MIN_WRITABLE_LOGDATE_NUM)
	{
		_isWritable.store(true);
	}
}

void LogManager::Record(std::string&& logDate)
{
	std::lock_guard<std::mutex> lock(_logMutex);//�̰߳�ȫ
	int curQue = _recordableQue.load();
	_logQue[curQue].emplace(std::move(logDate));
	if (_logQue[curQue].size() == MIN_WRITABLE_LOGDATE_NUM)
	{
		_isWritable.store(true);
	}
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
	Record(std::move(LogDataTypeStr[dataType] +  logData));
}

void LogManager::Record(unsigned dataType, std::string&& logData)
{
	Record(std::move(LogDataTypeStr[dataType] +  logData));
}

void LogManager::WriteDownLog()
{
	const int64_t kMaxPerLogFileByte = Parameter::maxLogDiskByte / 2;
	while (true)
	{
		if (_isWritable.load())
		{//ֻҪ��д���ͱ���Ҫд��
			//��־�����ȼ�¼����һ������
			_isWritable.store(false);
			int curWritingQue = _recordableQue.load();
			_recordableQue.store(!curWritingQue);

			//��ʼ��ӡ��ǰ��־���ݶ���
			if ((Parameter::maxLogQueueByte / 2) < (_logQue[curWritingQue].size() * BYTE_PER_LOGQUE_STRING) && _logQue[!curWritingQue].size())
			{
				//����ÿ��string��BYTE_PER_LOGQUE_STRING�ֽڹ��㣬����ǰ������־����ռ���ڴ�̫��
				//����ζ��RecordLog���ٶȺܿ죬��ĳ���ط������˾��ҵĴ�������ʱ������ֻ��ǰ��һЩ
				//��־�ǹؼ��ģ��������־���޹ؽ�Ҫ����������������ǰ���У��Է�ֹ�ڴ汬����ͬʱ׷��
				//RecordLog���ٶ�
				while (!_logQue[curWritingQue].empty())
				{
					_logQue[curWritingQue].pop();
				}
			}
			else
			{
				while (!_logQue[curWritingQue].empty())
				{
					if (_curLogFileByte > kMaxPerLogFileByte)
					{//����Ҫ�����ˣ������ɵ���־
						::fclose(_logFile);
						_logFileIdx = !_logFileIdx;
						_logFile = ::fopen((_logPath + std::to_string(_logFileIdx)).c_str(), "w");
						_curLogFileByte = 0;
					}
					std::string& buf = _logQue[curWritingQue].front();
					buf.append("\n");
					fwrite(buf.c_str(), buf.size(), 1, _logFile);
					_logQue[curWritingQue].pop();
					_curLogFileByte += BYTE_PER_LOGQUE_STRING;
				}
				fflush(_logFile);
			}
		}
		else if (_stop)
		{
			return;
		}
		else
		{//���ݲ���д���ȵ�һ��ʱ��
			std::this_thread::sleep_for(std::chrono::milliseconds(WRITING_LOG_WAIT_TIME));
		}
	}
}