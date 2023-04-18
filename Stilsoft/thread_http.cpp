#include "thread_http.h"

ThreadHttp::ThreadHttp(std::string &url)
{
	socketData.url = url;
	threadData.thread = new std::thread(&ThreadHttp::Thread, this);
	threadData.thread->detach();
	threadData.status = StatusThread::RUN;
}

ThreadHttp::~ThreadHttp()
{
	if (threadData.status != StatusThread::BROKEN)
	{
		threadData.status = StatusThread::STOP;
		while (threadData.status != StatusThread::COMPLETED)
			Sleep(100);
	}

	delete threadData.thread;
}

void ThreadHttp::ReadSocket()
{
	//Снятие блокировки сокета
	u_long mode(1);
	ioctlsocket(socketData.socket, FIONBIO, &mode);

	int sizeReadBuf{ 0 };
	BufNode tmpBufNode;
	for (;;)
	{
		sizeReadBuf = recv(socketData.socket, tmpBufNode.buf, SOCKET_BUF_SIZE, 0);
		if (sizeReadBuf <= 0)
			break;

		tmpBufNode.actualSize = sizeReadBuf;

		ThreadHttp::ArrayChar buf = std::make_unique<char[]>(sizeReadBuf);
		memcpy(buf.get(), tmpBufNode.buf, sizeReadBuf);
		PushRequest(buf);
	}
}

void ThreadHttp::SendRequest()
{
	std::string request{
"GET / HTTP/1.1\r\n\
Host: " + socketData.url + "\r\n\
User-Agent: Stilsoft\r\n\
Accept-Language: ru\r\n\
Accept-Charset: utf-8\r\n\
Connection: keep-alive\r\n\r\n"
};

	send(socketData.socket, request.c_str(), request.length(), 0);
}

void ThreadHttp::PushRequest(ThreadHttp::ArrayChar &buf)
{
	threadData.accesRequestBuf.lock();
	requestBuf.push_back(std::move(buf));
	threadData.accesRequestBuf.unlock();
}

ThreadHttp::ArrayChar ThreadHttp::PopRequest()
{
	ArrayChar result;
	threadData.accesRequestBuf.lock();
	if (requestBuf.empty() == false)
	{
		result = std::move(requestBuf.front());
		requestBuf.pop_front();
	}
	threadData.accesRequestBuf.unlock();

	return result;
}

void ThreadHttp::Thread(ThreadHttp* ts)
{
	if (ts->OpenSocket() == false)
	{
		ts->threadData.status = StatusThread::BROKEN;
		return;
	}

	while (ts->threadData.status == StatusThread::RUN)
	{
		ts->SendRequest();
		ts->ReadSocket();
		Sleep(DELAY_HTTP_REQUEST);
	}

	ts->CloseSocket();

	ts->threadData.status = StatusThread::COMPLETED;
}

bool ThreadHttp::OpenSocket()
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
		return false;

	socketData.socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	addrinfo hints;
	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	if (getaddrinfo(socketData.url.c_str(), "http", &hints, &socketData.addressInfo) != 0)
	{
		WSACleanup();
		return false;
	}

	if (connect(socketData.socket, socketData.addressInfo->ai_addr, sizeof(struct sockaddr)) != 0)
	{
		WSACleanup();
		return false;
	}

	return true;
}

void ThreadHttp::CloseSocket()
{
	shutdown(socketData.socket, 2);
	closesocket(socketData.socket);

	WSACleanup();
}