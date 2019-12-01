#pragma once

#include "utils.h"
#include "Parameter.h"

#include <arpa/inet.h>
#include <sys/types.h>
#include <string>
#include <unistd.h>
#include <sys/socket.h>

struct tcp_info;

namespace kikilib {

	//Socket�࣬������Socket����Ĭ�϶��Ƿ�����,�˿ڸ��õ�
	//ְ��
	//1���ṩfd���������API
	//2������fd����������
	//���������ü�������ĳһfdû�����˾ͻ�close
	class Socket
	{
	public:
		explicit Socket(int sockfd, std::string ip = "", int port = -1)
			: _sockfd(sockfd), _ip(std::move(ip)), _port(port)
		{
			_pRef = new int(1);
			if (sockfd > 0)
			{
				SetTcpNoDelay(Parameter::isNoDelay);
				SetNonBolckSocket();
				SetReuseAddr(true);
				SetReusePort(true);
			}
		}

		Socket()
			: _sockfd(::socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, IPPROTO_TCP)), _ip(""), _port(-1)
		{
			_pRef = new int(1);
			if (_sockfd > 0)
			{
				SetTcpNoDelay(Parameter::isNoDelay);
				SetReuseAddr(true);
				SetReusePort(true);
			}
		}

		Socket(const Socket& otherSock) : _sockfd(otherSock._sockfd)
		{
			*(otherSock._pRef) += 1;
			_pRef = otherSock._pRef;
			_ip = otherSock._ip;
			_port = otherSock._port;
		}

		Socket(Socket&& otherSock) : _sockfd(otherSock._sockfd)
		{
			*(otherSock._pRef) += 1;
			_pRef = otherSock._pRef;
			_ip = otherSock._ip;
			_port = otherSock._port;
		}

		Socket& operator=(const Socket& otherSock) = delete;

		~Socket();

		//���ص�ǰSocket��fd
		int fd() const { return _sockfd; }

		//��ip��port����ǰSocket
		void Bind(std::string& ip, int port);

		//��ʼ������ǰSocket
		void Listen();

		//��������һ�����ӣ�����һ�������ӵ�Socket
		Socket Accept();

		//��socket�ж�����
		ssize_t Read(void* buf, size_t count);

		//��socket��д����
		ssize_t Send(const void* buf, size_t count);

		//��ȡ��ǰ�׽��ֵ�Ŀ��ip
		std::string GetIp() { return _ip; }

		//��ȡ��ǰ�׽��ֵ�Ŀ��port
		int GetPort() { return _port; }

		//��ȡ�׽��ֵ�ѡ��,�ɹ��򷵻�true����֮������false
		bool GetSocketOpt(struct tcp_info*) const;

		//��ȡ�׽��ֵ�ѡ����ַ���,�ɹ��򷵻�true����֮������false
		bool GetSocketOptString(char* buf, int len) const;

		//��ȡ�׽��ֵ�ѡ����ַ���
		std::string GetSocketOptString() const;

		//�ر��׽��ֵ�д����
		void ShutdownWrite();

		//�����Ƿ���Nagle�㷨������Ҫ��������ݰ�����������ʱ���ܻ�����
		void SetTcpNoDelay(bool on);

		//�����Ƿ��ַ����
		void SetReuseAddr(bool on);

		//�����Ƿ�˿�����
		void SetReusePort(bool on);

		//�����Ƿ�ʹ���������
		void SetKeepAlive(bool on);

		//����socketΪ��������
		void SetNonBolckSocket();

		//����socketΪ������
		void SetBlockSocket();

		//void SetNoSigPipe();

	private:
		//fd
		const int _sockfd;

		//���ü���
		int* _pRef;

		//�˿ں�
		int _port;

		//ip
		std::string _ip;
	};

}  // namespace kikilib