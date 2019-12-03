#include "LogManager.h"
#include "Parameter.h"

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
	_condition.notify_one();
}

void LogManager::Record(std::string&& logDate)
{
	std::lock_guard<std::mutex> lock(_logMutex);//�̰߳�ȫ
	int curQue = _recordableQue.load();
	_logQue[curQue].emplace(std::move(logDate));
	_condition.notify_one();
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
		if (_stop)
		{
			return;
		}
		else
		{//ֻҪ��д���ͱ���Ҫд��
			//��־�����ȼ�¼����һ������
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
						fflush(_logFile);
						::fclose(_logFile);
						++_curLogFileIdx;
						_logFile = ::fopen((_logPath + std::to_string(_curLogFileIdx & 1)).c_str(), "w");
						_curLogFileByte = 0;
						//���ȼ�¼��ǰ��־Ϊ�ڼ�����־��
						std::string fileMsg = "cur file idx : " + std::to_string(_curLogFileIdx) + '\n';
						fwrite(fileMsg.c_str(), fileMsg.size(), 1, _logFile);
					}
					std::string& buf = _logQue[curWritingQue].front();
					buf.append("\n");
					fwrite(buf.c_str(), buf.size(), 1, _logFile);
					_logQue[curWritingQue].pop();
					_curLogFileByte += BYTE_PER_LOGQUE_STRING;
				}
				fflush(_logFile);
			}

			{//�����һ������û�����ˣ�������
				std::unique_lock<std::mutex> lock(_logMutex);
				if (_logQue[!curWritingQue].empty())
				{
					_condition.wait(lock);
				}
			}

		}
	}
}