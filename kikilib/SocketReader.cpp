//@Author Liu Yukang 
#include "SocketReader.h"
#include "Parameter.h"

#include <string.h>
#include <stdlib.h>

using namespace kikilib;

SocketReader::SocketReader(Socket& sock)
	: _sock(sock), _leftBorder(0), _rightBorder(0)
{
	_buffer.resize(Parameter::bufferInitLen);
}

SocketReader::SocketReader(Socket&& sock)
	: _sock(sock), _leftBorder(0), _rightBorder(0)
{
	_buffer.resize(Parameter::bufferInitLen);
}

bool SocketReader::IsEmptyAfterRead()
{
	if (_rightBorder == _leftBorder)
	{
		auto ret = ReadFillBuf();
		if (ret <= 0)
		{
			return true;
		}
	}
	return false;
}

//��ȡһ��int����������û�У��򷵻�false
bool SocketReader::ReadInt32(int& res)
{
	size_t newLeft = _leftBorder, numSize = 0;
	bool isPositive = true;
	if (newLeft < _rightBorder && (_buffer[newLeft] == '+' || _buffer[newLeft] == '-'))
	{//�����ţ��з���Ҫ��һ��
		isPositive = (_buffer[newLeft] == '+');
		++newLeft;
	}

	while(newLeft < _rightBorder && _buffer[newLeft + numSize] >= '0' && _buffer[newLeft + numSize] <= '9')
	{
		++numSize;
	}

	if (numSize == 0 || newLeft == _rightBorder)
	{//˵��û���ֻ��߶����˶���û���ַ������´�read���п��ܻ������֣���Ҫ�ȴ�һ�����ֽ�������ܳɹ�����
		return false;
	}
	_leftBorder = newLeft + numSize;
	res = std::strtol(&_buffer[newLeft],nullptr,10);
	if (!isPositive)
	{
		res = -res;
	}
	return true;
}

//��ȡһ��int64����������û�У��򷵻�false,�Ժ�ʵ��
//bool SocketReader::ReadInt64(int64_t& res)
//{
//	ReadFillBuf();
//
//}

//��ȡ����Ϊlen�����ݣ���û�г���Ϊlen�����ݣ��򷵻ؿմ�
std::string SocketReader::Read(size_t len)
{
	//�������ڷ��ش�����string��Ϊ������RVO����
	//buffer���Ѿ����㹻��������
	if (_rightBorder - _leftBorder >= len)
	{
		_leftBorder += len;
		return std::string(_buffer.begin() + _leftBorder - len, _buffer.begin() + _leftBorder);
	}
	if (_buffer.size() - _leftBorder < len)
	{//buffer�ռ䲻���Խ��ճ���len�����ݣ���չ���ոտ���
		_buffer.resize(len + _leftBorder);
	}
	ReadFillBuf();
	if (_rightBorder - _leftBorder >= len)
	{//��ȡ֮�����㹻��������
		_leftBorder += len;
		return std::string(_buffer.begin() + _leftBorder - len, _buffer.begin() + _leftBorder);
	}
	//��ȡ֮����û���㹻������
	return std::string("");
}

//��ȡ����Ϊlen�����ݣ���û�г���Ϊlen�����ݣ��򷵻�false
bool SocketReader::Read(char* buf, size_t len)
{
	//buffer���Ѿ����㹻��������
	if (_rightBorder - _leftBorder >= len)
	{
		memcpy(buf,&_buffer[_leftBorder],len);
		_leftBorder += len;
		return true;
	}
	if (_buffer.size() - _leftBorder < len)
	{//buffer�ռ䲻���Խ��ճ���len�����ݣ���չ���ոտ���
		_buffer.resize(len + _leftBorder);
	}
	ReadFillBuf();
	if (_rightBorder - _leftBorder >= len)
	{//��ȡ֮�����㹻��������
		memcpy(buf, &_buffer[_leftBorder], len);
		_leftBorder += len;
		return true;
	}
	//��ȡ֮����û���㹻������
	return false;
}

//���Զ�ȡ������������������,���������Ѿ����ˣ���������Parameter::bufExpandRatio����С
ssize_t SocketReader::ReadFillBuf()
{
	if (_leftBorder >= (_buffer.size() / Parameter::bufMoveCriterion))
	{//����ߵĿ���λ��̫��
		memmove(&(_buffer.front()), &(_buffer[_leftBorder]), _rightBorder - _leftBorder);
		_rightBorder -= _leftBorder;
		_leftBorder = 0;
	}

	if (_rightBorder == _buffer.size())
	{//������������
		_buffer.resize(static_cast<size_t>(static_cast<double>(_buffer.size())* Parameter::bufExpandRatio));
	}

	ssize_t ret = _sock.Read(&(_buffer[_rightBorder]), _buffer.size() - _rightBorder);

	if (ret > 0)
	{
		_rightBorder += ret;
	}
	return ret;
}

//��ȡ�����ܶ�ȡ�����ݣ�û���򷵻ؿմ�
std::string SocketReader::ReadAll()
{
	//һֱ����û��������
	while (ReadFillBuf() > 0)
	{
		if (_rightBorder != _buffer.size())
		{
			break;
		}
	}
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder;
	_rightBorder = 0;
	_leftBorder = 0;
	return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + tmpRight);
}

//��һ�У�������\r\n��β
std::string SocketReader::ReadLineEndOfRN()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\r' && endPos + 1 < tmpRight && _buffer[endPos + 1] == '\n')
		{
			//�ҵ�\r\n
			endPos += 2;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//û��\r\n
	//_rightBorder == _buffer.size()��ζ�ſ��ܻ���socket����û����
	if (_rightBorder == _buffer.size() && ReadFillBuf() > 0)
	{	
		//socket��������������û���꣬�ٴγ���Ѱ��\r\n
		return ReadLineEndOfRN();
	}
	//socket�����˶�û����\r\n
	return std::string("");
}

//��һ�У�������\r��β
std::string SocketReader::ReadLineEndOfR()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\r')
		{
			//�ҵ�\r
			++endPos;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//û��\r
	//_rightBorder == _buffer.size()��ζ�ſ��ܻ���socket����û����
	if (_rightBorder == _buffer.size() && ReadFillBuf() > 0)
	{
		//socket��������������û���꣬�ٴγ���Ѱ��\r
		return ReadLineEndOfR();
	}
	//socket�����˶�û����\r
	return std::string("");
}

//��һ�У�������\n��β
std::string SocketReader::ReadLineEndOfN()
{
	//ReadFillBuf();
	size_t tmpLeft = _leftBorder, tmpRight = _rightBorder, endPos = tmpLeft;
	for (; endPos < tmpRight; ++endPos)
	{
		if (_buffer[endPos] == '\n')
		{
			//�ҵ�\n
			++endPos;
			_leftBorder = endPos;
			return std::string(_buffer.begin() + tmpLeft, _buffer.begin() + endPos);
		}
	}
	//û��\n
	//_rightBorder == _buffer.size()��ζ�ſ��ܻ���socket����û����
	if (_rightBorder == _buffer.size() && ReadFillBuf() > 0)
	{
		//socket��������������û���꣬�ٴγ���Ѱ��\r
		return ReadLineEndOfR();
	}
	//socket�����˶�û����\n
	return std::string("");
}
