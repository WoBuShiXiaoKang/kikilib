#pragma once

//#include <exception>
#include <queue>
#include <mutex>
#include <atomic>
#include <string>
#include <stdio.h>
#include <thread>
#include <condition_variable>

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
#define StartLogMgr(logPath) (kikilib::LogManager::GetLogMgr()->StartLogManager((logPath)))
//�ر���־������
#define EndLogMgr() (kikilib::LogManager::GetLogMgr()->EndLogManager())
//��¼��־��ʹ�øú꼴��
#define RecordLog(...) (kikilib::LogManager::GetLogMgr()->Record(__VA_ARGS__))

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
		LogManager() {
			_stop = false;
			_isInit = false;
			_recordableQue.store(0);
			_curLogFileByte = 0;
			_curLogFileIdx = 0;
		};

		~LogManager() {};

	public:

		//��־�������಻������
		DISALLOW_COPY_MOVE_AND_ASSIGN(LogManager);

		//��ʼ��LogManager�����÷���Ϊkikilib::LogManager::GetLogMgr()->InitLogManager(path)
		//�ڳ�������ʱ�����ȵ��øú�������������ʹ�����쳣��
		bool StartLogManager(std::string logPath);

		//������־���������رս���ǰ����ִ��
		void EndLogManager();

		//��ȡ��־�������ʵ��
		//��û�г�ʼ���򷵻�nullptr
		static LogManager* GetLogMgr();

		//��¼��־
		void Record(const char* logData);
		void Record(std::string& logData);
		void Record(std::string&& logData);
		void Record(unsigned dataType, const char* logData);
		void Record(unsigned dataType, std::string& logData);
		void Record(unsigned dataType, std::string&& logData);

	private:
		//�����д����־
		void WriteDownLog();

	private:
		//��־������ʵ��
		static LogManager* _logMgr;

		//���ڱ���������Ϊ�˷�����ִ��Ч�ʣ�ԭ���ϲ�������ռ�д���
		static std::mutex _logMutex;

		//�������������ʹ�õ���
		std::mutex _conditionMutex;

		//���ڻ���д��־�߳�
		std::condition_variable _condition;

		//�Ƿ��Ѿ���ʼ��
		static bool _isInit;

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

		//�������У�һ�������߳�ȡ����д��־����һ���������throw�������쳣
		//���߳�д��־��ն��к��޸�_recordableQue��ֵ��Ȼ�������һ�����н���д��־
		std::queue<std::string> _logQue[2];

		//��ǰ�����ڴ�����д���ݵĶ���
		std::atomic_int _recordableQue;

		bool _stop;
	};
}
