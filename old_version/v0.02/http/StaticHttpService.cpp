//@Author Liu Yukang 
#include "StaticHttpService.h"

#include <stdio.h>
#include <sys/types.h>
#include <unistd.h>
#include <ctype.h>
#include <strings.h>
#include <string.h>
#include <sys/stat.h>
//#include <pthread.h>
#include <sys/wait.h>
#include <stdlib.h>

static std::string rootPath(".");

void StaticHttpService::HandleReadEvent()
{
	struct stat st;

	//��http ����ĵ�һ�����ݣ�request line���������󷽷���� method ��
	std::string line = ReadLineEndOfRN();
	int lineN = line.size();
	if (lineN <= 0)
	{//�ͻ��˻�û����
		return;
	}
	int methodEnd = 0;
	for (; methodEnd < lineN; ++methodEnd)
	{
		if (isspace(static_cast<int>(line[methodEnd])))
		{
			break;
		}
	}
	std::string method(line.begin(), line.begin() + methodEnd);

	//�������ķ������� GET �Ļ���ֱ�ӷ��� response ���߿ͻ���ûʵ�ָ÷���
	if (method != "GET")
	{
		//����ʣ�µ�����
		ReadAll();
		SendUnImpletement();
		return;
	}

	//�������еĿհ��ַ�(�ո�)
	int urlStart = methodEnd + 1, urlEnd = urlStart;
	for (; urlStart < lineN ; ++urlStart)
	{
		if (!isspace(static_cast<int>(line[urlStart])))
		{
			break;
		}
	}

	//��ȡurl
	for (urlEnd = urlStart; urlEnd < lineN; ++urlEnd)
	{
		if (isspace(static_cast<int>(line[urlEnd])))
		{
			break;
		}
	}

	//��urlƴ����htdocs����
	std::string path =rootPath + std::string(line.begin() + urlStart, line.begin() + urlEnd);

	//��� path �����е�����ַ��������һ���ַ������ַ� / ��β�Ļ�����ƴ����һ��"index.html"���ַ�������ҳ����˼
	if (path.back() == '/')
	{
		path += "index.html";
	}
	//����ʣ�µ�����
	ReadAll();
	//��ϵͳ��ȥ��ѯ���ļ��Ƿ����
	if (stat(path.c_str(), &st) == -1)
	{//��������ڣ�����һ���Ҳ����ļ��� response ���ͻ���
		SendNotFount();
	}
	else
	{//�ļ����ڣ���ȥ������S_IFMT���룬����֮���ֵ���������жϸ��ļ���ʲô���͵�
		if ((st.st_mode & S_IFMT) == S_IFDIR)
		{//�������ļ��Ǹ�Ŀ¼���Ǿ���Ҫ���� path ����ƴ��һ��"/index.html"���ַ���
			path += "/index.html";
		}

		//if ((st.st_mode & S_IXUSR) || (st.st_mode & S_IXGRP) || (st.st_mode & S_IXOTH))
		//{//�������ļ���һ����ִ���ļ�������
		  //  int a = 0;
		//}
		//else
		{
			SendFile(path);
		}
	}
	Close();
}

void StaticHttpService::HandleErrEvent()
{
	Close();
}

void StaticHttpService::SendUnImpletement()
{
	if(!WriteBuf("HTTP/1.0 501 Method Not Implemented\r\n")) return;
	if(!WriteBuf("Server: kikihttp/0.1.0\r\n")) return;
	if(!WriteBuf("Content-Type: text/html\r\n")) return;
	if(!WriteBuf("\r\n")) return;
	if(!WriteBuf("<HTML><TITLE>Method Not Implemented\r\n")) return;
	if(!WriteBuf("</TITLE></HEAD>\r\n")) return;
	if(!WriteBuf("<BODY><P>HTTP request method not supported.\r\n")) return;
	if(!WriteBuf("</BODY></HTML>\r\n")) return;
}

void StaticHttpService::SendFile(std::string& path)
{
	FILE* fp = NULL;

	//����������������·����ָ���ļ�
	fp = fopen(path.c_str(), "r");
	if (fp == NULL)
	{
		SendNotFount();
	}
	else
	{
		//�򿪳ɹ��󣬽�����ļ��Ļ�����Ϣ��װ�� response ��ͷ��(header)������
		SendHeader(path);
		//���Ű�����ļ������ݶ�������Ϊ response �� body ���͵��ͻ���
		SendBody(fp);

		fclose(fp);
	}
	
}

void StaticHttpService::SendNotFount()
{
	if(!WriteBuf("HTTP/1.0 404 NOT FOUND\r\n")) return;
	if(!WriteBuf("Server: kikihttp/0.1.0\r\n")) return;
	if(!WriteBuf("Content-Type: text/html\r\n")) return;
	if(!WriteBuf("\r\n")) return;
	if(!WriteBuf("<HTML><TITLE>Not Found</TITLE>\r\n")) return;
	if(!WriteBuf("<BODY><P>The server could not fulfill\r\n")) return;
	if(!WriteBuf("your request because the resource specified\r\n")) return;
	if(!WriteBuf("is unavailable or nonexistent.\r\n")) return;
	if(!WriteBuf("</BODY></HTML>\r\n")) return;
}

void StaticHttpService::SendHeader(std::string& path)
{
	//path��׺���Եõ��ļ����ͣ�̫���ˣ��Ժ��п���д
	if(!WriteBuf("HTTP/1.0 200 OK\r\n")) return;
	if(!WriteBuf("Server: kikihttp/0.1.0\r\n")) return;
	if(!WriteBuf("Content-Type: text/html\r\n")) return;
	if(!WriteBuf("\r\n")) return;
}

void StaticHttpService::SendBody(FILE* fp)
{
	char buf[1024];
	char* ret;
	//���ļ��ļ��������ж�ȡָ������
	ret = fgets(buf, sizeof(buf), fp);
	while (!feof(fp))
	{
		if (!WriteBuf(std::string(buf))) return;
		ret = fgets(buf, sizeof(buf), fp);
	}
}

