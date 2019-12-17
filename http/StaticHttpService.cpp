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

void StaticHttpService::handleReadEvent()
{
	struct stat st;

	//��http ����ĵ�һ�����ݣ�request line���������󷽷���� method ��
	std::string line = readLineEndOfRN();
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
		readAll();
		sendUnImpletement();
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
	readAll();
	//��ϵͳ��ȥ��ѯ���ļ��Ƿ����
	if (stat(path.c_str(), &st) == -1)
	{//��������ڣ�����һ���Ҳ����ļ��� response ���ͻ���
		sendNotFount();
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
			sendFile(path);
		}
	}
	forceClose();
}

void StaticHttpService::handleErrEvent()
{
	forceClose();
}

void StaticHttpService::sendUnImpletement()
{
	if (!sendContent("HTTP/1.0 501 Method Not Implemented\r\n")) return;
	if (!sendContent("Server: kikihttp/0.1.0\r\n")) return;
	if (!sendContent("Content-Type: text/html\r\n")) return;
	if (!sendContent("\r\n")) return;
	if (!sendContent("<HTML><TITLE>Method Not Implemented\r\n")) return;
	if (!sendContent("</TITLE></HEAD>\r\n")) return;
	if (!sendContent("<BODY><P>HTTP request method not supported.\r\n")) return;
	if (!sendContent("</BODY></HTML>\r\n")) return;
}

void StaticHttpService::sendFile(std::string& path)
{
	FILE* fp = NULL;

	//����������������·����ָ���ļ�
	fp = fopen(path.c_str(), "r");
	if (fp == NULL)
	{
		sendNotFount();
	}
	else
	{
		//�򿪳ɹ��󣬽�����ļ��Ļ�����Ϣ��װ�� response ��ͷ��(header)������
		sendHeader(path);
		//���Ű�����ļ������ݶ�������Ϊ response �� body ���͵��ͻ���
		sendBody(fp);

		fclose(fp);
	}
	
}

void StaticHttpService::sendNotFount()
{
	if(!sendContent("HTTP/1.0 404 NOT FOUND\r\n")) return;
	if(!sendContent("Server: kikihttp/0.1.0\r\n")) return;
	if(!sendContent("Content-Type: text/html\r\n")) return;
	if(!sendContent("\r\n")) return;
	if(!sendContent("<HTML><TITLE>Not Found</TITLE>\r\n")) return;
	if(!sendContent("<BODY><P>The server could not fulfill\r\n")) return;
	if(!sendContent("your request because the resource specified\r\n")) return;
	if(!sendContent("is unavailable or nonexistent.\r\n")) return;
	if(!sendContent("</BODY></HTML>\r\n")) return;
}

void StaticHttpService::sendHeader(std::string& path)
{
	//path��׺���Եõ��ļ����ͣ�̫���ˣ��Ժ��п���д
	if(!sendContent("HTTP/1.0 200 OK\r\n")) return;
	if(!sendContent("Server: kikihttp/0.1.0\r\n")) return;
	if(!sendContent("Content-Type: text/html\r\n")) return;
	if(!sendContent("\r\n")) return;
}

void StaticHttpService::sendBody(FILE* fp)
{
	char buf[1024];
	char* ret;
	//���ļ��ļ��������ж�ȡָ������
	ret = fgets(buf, sizeof(buf), fp);
	while (!feof(fp))
	{
		if (!sendContent(std::string(buf))) return;
		ret = fgets(buf, sizeof(buf), fp);
	}
}

