//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "Parameter.h"
#include "utils.h"

#include <sys/types.h>
#include <vector>
#include <string>
#include <stdint.h>

namespace kikilib
{
	class EventService;

	//socket��д��
	//ְ��
	//1���ṩдsocket���ͻ��������ݵ�API
	//2�����ṩһ�����û������������û�һ����Ϊ����ԭ��ûд�������
	class SocketWtitter
	{
	public:
		SocketWtitter(Socket& sock, EventService* pEvServe);
		SocketWtitter(Socket&& sock, EventService* pEvServe);

		~SocketWtitter() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(SocketWtitter);

		//����һ��int
		void SendInt32(int res);

		//����һ���ַ���
		void Send(std::string& str);

		//����һ���ַ���
		void Send(std::string&& str);

		//������������д��socket��
		void WriteBufToSock();

	private:

		Socket _sock;

		//������������߽�
		size_t _leftBorder;

		//�����������ұ߽�
		size_t _rightBorder;

		//�����������¼�����
		EventService* _pEvServe;

		//��������
		std::vector<char> _buffer;

	};

}