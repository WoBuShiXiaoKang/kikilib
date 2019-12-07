//@Author Liu Yukang 
#pragma once

#include "Socket.h"
#include "SocketReader.h"
#include "SocketWritter.h"
#include "Time.h"
#include "utils.h"

#include <poll.h>
#include <sys/epoll.h>
#include <unistd.h>
#include <functional>
#include <future>

namespace kikilib
{
	class EventManager;

	//�¼�����Ա��
	//ְ��
	//1��ר����һ��socket��רע�ڷ����socket�Ϸ������¼�
	//2���ṩ����socket�Ĳ���API
	//3���ṩ�����¼���صĲ���API
	//4���ṩ��ʱ����صĲ���API
	//5���ṩ�̳߳ع��ߵĲ���API
	//6���ṩsocket�������Ķ�д����API
	//ʹ�÷�����
	//1���û��̳и��࣬ʵ�����е�HandleConnectionEvent(),
	//   HandleReadEvent(),HandleErrEvent,HandleCloseEvent()
	//   �������ɣ����Զ����Լ���˽�г�Ա�������˴���������
	//   �е�context������ָ�룬����������Ҳ������
	//2���û��̳�EventServiceFactory�࣬ʵ��һ�������ö���ķ���
	//   Ȼ�󽫹�����ʵ������EventMaster��������������ת��ÿ��һ��
	//   �µ����ӵ�����EventMaster�ͻ�ʹ�ù�������һ�����¼�����Ϊ
	//   �µ����ӷ���
	class EventService
	{
	public:
		/////////////////////////////////////////////////////////////////////////////////////////////////////////
		////////////////////////////////////////////�û�API//////////////////////////////////////////////////////

		//////////////////////////////����socket�Ĳ���API//////////////////////////////////

		int fd() { return _sock.fd(); };

		std::string peerIP() { return _sock.GetIp(); };

		int peerPort() { return _sock.GetPort(); };

		//��ȡ�׽��ֵ�ѡ����ַ���
		std::string GetSocketOptString() { return _sock.GetSocketOptString(); };

		//�رյ�ǰ�¼�
		void Close();

		///////////////////////////////�����¼��Ĳ���API///////////////////////////////////

		//��ȡ��ǰ�¼���������ע���¼�
		int GetInteresEv() { return _interestEvent; };
		
		//�޸ĵ�ǰ�¼���������ע���¼�
		void SetInteresEv(int newInterestEv);

		//��ȡ��ǰ�¼������ȼ�
		unsigned GetEventPriority() { return _eventPriority; };

		//�޸ĵ�ǰ�¼������ȼ�
		void SetEventPriority(unsigned priority) { _eventPriority = priority; };

		///////////////////////////////��ʱ����صĲ���API///////////////////////////////////

		//timeʱ���ִ��timerCb����
		void RunAfter(Time time, std::function<void()>&& timerCb);

		//ÿ��timeʱ��ִ��timerCb����
		void RunEvery(Time time, std::function<void()> timerCb);

		//ÿ��timeʱ��ִ��һ��timerCb����,ֱ��isContinue��������false
		void RunEveryUntil(Time time, std::function<void()> timerCb, std::function<bool()> isContinue);

		///////////////////////////////�̳߳���صĲ���API///////////////////////////////////

		//�������������̳߳����Դﵽ�첽ִ�е�Ч�����磺
		//1�������ݿ�д���ݣ�ֱ�ӽ�д���ݿ�ĺ�����������
		//2�������ݿ�����ݣ�����ȡ���ݿ�ĺ����������У�
		//   Ȼ�����ö�ʱ���¼�����timeʱ������Ƿ����
        void RunInThreadPool(std::function<void()>&& func);

		////////////////////////////socket��������صĲ���API/////////////////////////////////

		//��socketдһ��int����num
		void WriteInt32(int num);
		
		//��socketд����content
		void WriteBuf(std::string& content);
		void WriteBuf(std::string&& content);

		//��ȡһ��int����������û�У��򷵻�false
		bool ReadInt32(int& res);

		//��ȡ����Ϊlen���ַ�
		std::string ReadBuf(size_t len);

		//��һ�У�������\r\n��β,��û�У����ؿմ�
		std::string ReadLineEndOfRN();

		//��һ�У�������\r��β,��û�У����ؿմ�
		std::string ReadLineEndOfR();

		//��һ�У�������\n��β,��û�У����ؿմ�
		std::string ReadLineEndOfN();

		//��ȡ�������е������ַ�
		std::string ReadAll();

		////////////////////////////////////////////////�û�API/////////////////////////////////////////////////
		////////////////////////////////////////////////////////////////////////////////////////////////////////
		 

		//////////////////////////���нӿ�,��Ҫ�û������Լ�ʵ��///////////////////////////
		//�µ����ӵ���ʱִ�еĺ���
		virtual void HandleConnectionEvent() {};

		//�ɶ��¼�����ʱִ�еĺ���
		virtual void HandleReadEvent() {};

		//�����¼�����ʱִ�еĺ���
		virtual void HandleErrEvent() {};

		//���ӹر�ʱִ�еĻص�����
		virtual void HandleCloseEvent() {};

		///////////////////////////��EventMaster��EventManager����////////////////////////////////

		//�����¼�״̬����ɶ�����д������
		void SetEventState(int state) { _eventState = state; };

		//EventManager����øú��������¼����ʹ����¼�
		//�û�������д����������
		virtual void HandleEvent();

		//�����Ƿ��Ѿ��ر�
		bool IsConnected() { return _isConnected; }

	public:
		EventService(Socket sock, EventManager* evMgr, int interestEvent = EPOLLIN | EPOLLPRI | EPOLLRDHUP);

		virtual ~EventService() {};

		DISALLOW_COPY_MOVE_AND_ASSIGN(EventService);

	private:
		//д�¼�
		void HandleWriteEvent();

		//��ע���¼�������EventBody��epoll�лᱻʲô�¼�����
		int _interestEvent;

		//�¼�״̬����д�¼����ɶ��¼���
		int _eventState;

		//�¼����ȼ���Ĭ��ΪNORMAL_EVENT
		unsigned _eventPriority;

		//�Ƿ��Ѿ�����
		bool _isConnected;

		//���¼������socket
		Socket _sock;

		//���¼��������¼�������
		EventManager* _pMyEvMgr;

		SocketWtitter _bufWritter;

		SocketReader _bufReader;
	};

}