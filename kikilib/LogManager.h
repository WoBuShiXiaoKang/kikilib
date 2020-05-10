//@Author Liu Yukang 
#pragma once

//#include <exception>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <stdio.h>
#include <thread>
#include <condition_variable>
#include <array>

#include "Time.h"
#include "Parameter.h"
#include "Sequence.h"
#include "utils.h"
//#include <stdarg.h>

//��־��Ϣ����
enum LogDataType
{
	DEBUG_DATA_INFORMATION = 0, //��¼��ͨ��Ϣ
	WARNING_DATA_INFORMATION,  //��¼������Ϣ
	ERROR_DATA_INFORMATION, //��¼���д���
};

////////////////////////////////////ȫ��ʹ�õ�API/////////////////////////////////
//����־������
#define StartLogMgr(logPath) (kikilib::LogManager::getLogMgr()->startLogManager((logPath)))
//�ر���־������
#define EndLogMgr() (kikilib::LogManager::getLogMgr()->endLogManager())
//��¼��־��ʹ�øú꼴��
//ע�⣬Ϊ��Ч�ʣ����е���־���ݶ���ת�Ƶģ���������ú�����ԭʼstring���ݻ�û���ˣ��û�����Ҫ������Ҫ�Լ�����һ��
#define RecordLog(...) (kikilib::LogManager::getLogMgr()->recordInLog(__VA_ARGS__))

namespace kikilib
{
	//��ӦLogDataType�ĺ�
	static std::string LogDataTypeStr[3] =
	{
		"remark : ", //��¼��ͨ��Ϣ
		"warning : ",  //��¼������Ϣ
		"error : ", //��¼���д���
	};

	//��־�������࣬����Ϊ������ʹ��˫������кͶ�����д��־�߳�ʵ��
	//ְ��
	//����������־��Ϣ��Ҫд��ʱ�򣬽���д����־�ļ��У�����˯��һ��ʱ��
	//ʹ�÷�����
	//1����������ʱ�����StartLogMgr()
	//2���������ǰ����EndLogMgr()
	//3��ʹ��RecordLog()������¼��־��Ϣ
	class LogManager
	{
	protected:
		LogManager();

		~LogManager() {};

	public:

		DISALLOW_COPY_MOVE_AND_ASSIGN(LogManager);

		//��ʼ��LogManager�����÷���Ϊkikilib::LogManager::GetLogMgr()->InitLogManager(path)
		//�ڳ�������ʱ�����ȵ��øú�������������ʹ�����쳣��
		bool startLogManager(std::string logPath);

		//������־���������رս���ǰ����ִ��
		void endLogManager();

		//��ȡ��־�������ʵ��
		//��û�г�ʼ���򷵻�nullptr
		static LogManager* getLogMgr();

		//��¼��־
		void recordInLog(const char* logData);
		void recordInLog(std::string& logData);
		void recordInLog(std::string&& logData);
		void recordInLog(unsigned dataType, const char* logData);
		void recordInLog(unsigned dataType, std::string& logData);
		void recordInLog(unsigned dataType, std::string&& logData);

	private:
		//�����д����־
		void writeDownLog();

		//������־ϵͳ��ʱ��
		void updateLogTime();

	private:
		//��־������ʵ��
		static LogManager* _logMgr;

		//���ڱ���������Ϊ�˷�����ִ��Ч�ʣ�ԭ���ϲ�������ռ�д���
		static std::mutex _logMutex;

		//�Ƿ��Ѿ���ʼ��
		static bool _isInit;

		//���ڻ���д��־�߳�
		std::condition_variable _condition;

		//��ǰ����д����־�ļ����ֽڴ�С
		int64_t _curLogFileByte;

		//��ǰ����д����־�ļ�����д�ĵڼ�����־�ļ���ʵ����Զֻ���������ļ������������������ɵ�һ������д
		int64_t _curLogFileIdx;

		//��־�ļ�·��
		std::string _logPath;

		//��־�ļ�,����֤����ofstream���죬��ʹ��fwrite
		FILE* _logFile;

		//д��־���߳�
		std::thread* _logLoop;

		Sequence _stop;

		//���һ���Ѷ�����λ��
		Sequence _lastRead;

		//���һ����д����λ��
		Sequence _lastWrote;

		//��ǰ��д�Ĳ�λ���
		AtomicSequence _writableSeq;

		std::mutex _timeMutex;

		std::string _logTimeStr;

		time_t _logTimeSec;

		//ʱ��ƫ��,��λΪ��
		long _timeZone;

		std::array<std::string, Parameter::kLogBufferLen> _ringBuf;
	};
}
