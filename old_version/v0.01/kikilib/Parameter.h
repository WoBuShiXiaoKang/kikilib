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
		//const int64_t maxLogQueueByte = 1073741824; //1024 * 1024 * 1024 * 1, 1GB

		//��־��ռ���̵�����С��Byte�������ڸ�����������һ����־�ļ������¿�ʼд����ֹ���̱���
		const int64_t maxLogDiskByte = 1073741824; //1024 * 1024 * 1024, 1GB

		//��־�е�ringbuffer�ĳ��ȣ���Ҫ��2��n����,һ��std::string��С��40�ֽڣ������ַ�������60�ֽڣ�
		//���Ը�����*100������Ϊ���ڴ�����ָ���־ϵͳ�Ĵ�С����4194304��ʾ�ڴ�������־419MB�ռ�
		const int64_t kLogBufferLen = 4194304;

		static_assert(((kLogBufferLen > 0) && ((kLogBufferLen& (~kLogBufferLen + 1)) == kLogBufferLen)),
			"RingBuffer's size must be a positive power of 2");

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