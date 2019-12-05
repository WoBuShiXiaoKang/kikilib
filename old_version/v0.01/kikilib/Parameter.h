//@Author Liu Yukang 
#pragma once

#include <string>
#include <string.h>

namespace kikilib
{
	//�����б�
	namespace Parameter
	{
		//��־����
		const static std::string logName("Log.txt");

		//��־��ռ�ڴ������С��Byte�������ڸ�����������һ������־���ݣ���ֹ�ڴ汬��
		const int64_t maxLogQueueByte = 1073741824; //1024 * 1024 * 1024 * 1, 1GB

		//��־��ռ���̵�����С��Byte�������ڸ�����������һ����־�ļ������¿�ʼд����ֹ���̱���
		const int64_t maxLogDiskByte = 21474836480; //20 * 1024 * 1024 * 1024, 20GB

		//�̳߳����̵߳�����
		constexpr static unsigned threadPoolCnt = 4;

		//�������еĳ���
		constexpr static unsigned backLog = 1024;

		//SocketĬ���Ƿ���TCP_NoDelay
		constexpr static bool isNoDelay = true;

		//��ȡ��Ծ��epoll_event������ĳ�ʼ����
		static constexpr int epollEventListFirstSize = 16;

		//epoll_wait������ʱ��
		static constexpr int epollTimeOutMs = 10000;

		//SocketReader��SocketWritter�л������ĳ�ʼ��С
		static constexpr size_t bufferInitLen = 1024;

		//SocketReader��SocketWritter�еĻ�������
		//��ǰ����е�λ�ô�����buffer�ܴ�С��1 / bufMoveCriterion��
		//���Զ���ǰ����
		static constexpr size_t bufMoveCriterion = 3;

		//SocketReader��SocketWritter�еĻ�������
		//���Ѿ����ˣ�������Ҫ�����ݣ�������չbuffer��sizeΪ��ǰ��bufExpandRatio��
		static constexpr double bufExpandRatio = 1.5;
	};
}