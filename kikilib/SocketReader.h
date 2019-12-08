//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "utils.h"

#include <sys/types.h>
#include <vector>
#include <string>
#include <stdint.h>

namespace kikilib
{
	//socket��ȡ��
	//ְ��
	//1���ṩ��ȡsocket���ջ��������ݵ�API
	//2�����ṩһ�����û������������û���û��������
	class SocketReader
	{
	public:
		SocketReader(Socket& sock);
		SocketReader(Socket&& sock);

		~SocketReader() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(SocketReader);

		//�������Ƿ�Ϊ��
		bool IsEmptyAfterRead();

		//��ȡһ��int����������û�У��򷵻�false
		bool ReadInt32(int& res);

		//��ȡһ��int64����������û�У��򷵻�false��δ��ʵ��
		//bool ReadInt64(int64_t& res);

		//��ȡ����Ϊlen�����ݣ���û�г���Ϊlen�����ݣ��򷵻ؿմ�
		std::string Read(size_t len);

		//��һ�У�������\r\n��β,��û�У����ؿմ�
		std::string ReadLineEndOfRN();

		//��һ�У�������\r��β,��û�У����ؿմ�
		std::string ReadLineEndOfR();

		//��һ�У�������\n��β,��û�У����ؿմ�
		std::string ReadLineEndOfN();

		//��ȡ�����ܶ�ȡ�����ݣ�û���򷵻ؿմ�
		std::string ReadAll();
		

	private:

		//���Զ�ȡ������������������,���������Ѿ����ˣ���������1.5����С
		ssize_t ReadFillBuf();

		Socket _sock;

		//������������߽�
		size_t _leftBorder;

		//�����������ұ߽�
		size_t _rightBorder;

		//��������
		std::vector<char> _buffer;

	};

}