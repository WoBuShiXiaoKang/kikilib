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