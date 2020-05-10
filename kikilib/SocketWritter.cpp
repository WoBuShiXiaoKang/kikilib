//@Author Liu Yukang 
#include "SocketWritter.h"
#include "EventService.h"
#include "LogManager.h"

#include <string.h>
#include <sys/epoll.h>

using namespace kikilib;

SocketWtitter::SocketWtitter(Socket& sock, EventService* pEvServe)
	: _sock(sock), _leftBorder(0), _rightBorder(0), _pEvServe(pEvServe)
{
	_buffer.resize(Parameter::bufferInitLen);
}

SocketWtitter::SocketWtitter(Socket&& sock, EventService* pEvServe)
	: _sock(sock), _leftBorder(0), _rightBorder(0), _pEvServe(pEvServe)
{
	_buffer.resize(Parameter::bufferInitLen);
}

//дһ��int
bool SocketWtitter::sendInt32(int res)
{
	return send(std::move(std::to_string(res)));
}

//дһ���ַ���
bool SocketWtitter::send(std::string& str)
{
	return send(std::move(str));//Ŀǰ������һ���ģ���û��ת��ָ��
}

//дһ���ַ���
bool SocketWtitter::send(std::string&& str)
{
	writeBufToSock();
	if (_rightBorder - _leftBorder)
	{//һ��ûд��
		if (_buffer.size() - _rightBorder < str.size())
		{
			_buffer.resize(_rightBorder + str.size());
		}
		memmove(&(_buffer[_rightBorder]), &(*str.begin()), str.size());
	}
	else
	{
		int ret = static_cast<int>(_sock.send(&(*str.begin()), str.size()));
		if (ret < 0)
		{//error
			RecordLog(ERROR_DATA_INFORMATION, "write socket error!");
			return false;
		}
		else if (ret < static_cast<int>(str.size()))
		{//����û����
			_pEvServe->setInteresEv(_pEvServe->getInteresEv() | EPOLLOUT);
			auto leftLen = str.size() - static_cast<size_t>(ret);
			if (_buffer.size() - _rightBorder < leftLen)
			{
				_buffer.resize(_rightBorder + leftLen);
			}
			memmove(&(_buffer[_rightBorder]), &(*(str.begin() + ret)), leftLen);
		}
		else
		{//һ�γɹ������������ݳ�ȥ
			return true;
		}
	}
	return true;
}

//������������д��socket��
void SocketWtitter::writeBufToSock()
{
	int curLen = _rightBorder - _leftBorder;
	if (!curLen)
	{//û�ж���д�ˣ�ȡ����ע���¼���
		return;
	}
	int ret = static_cast<int>(_sock.send(&(_buffer[_leftBorder]), curLen));
	if (ret < 0)
	{
		RecordLog(ERROR_DATA_INFORMATION, "write socket error!");
		return;
	}
	else if (ret < curLen)
	{//ûд��
		_pEvServe->setInteresEv(_pEvServe->getInteresEv() | EPOLLOUT);
	}
	_leftBorder += ret;

	if (_leftBorder >= (_buffer.size() / Parameter::bufMoveCriterion))
	{
		memmove(&(_buffer.front()), &(_buffer[_leftBorder]), _rightBorder - _leftBorder);
		_rightBorder -= _leftBorder;
		_leftBorder = 0;
	}
}
