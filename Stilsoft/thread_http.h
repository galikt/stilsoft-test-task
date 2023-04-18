#ifndef THREAD_HTTP
#define THREAD_HTTP

#include <iostream>
#include <thread>
#include "Winsock2.h"
#include <ws2tcpip.h>
#include <Windows.h>
#include <string.h>
#include <deque>
#include <vector>
#include <mutex>
#include <memory>

#pragma comment (lib, "Ws2_32.lib")

const int SOCKET_BUF_SIZE{ 128 };
const int DELAY_HTTP_REQUEST{ 500 };

class ThreadHttp
{
public:
	using ArrayChar = std::unique_ptr<char[]>;
	ThreadHttp(std::string& url);
	~ThreadHttp();

	ThreadHttp::ArrayChar PopRequest();

private:
	enum StatusThread { RUN, STOP, BROKEN, COMPLETED };

	struct
	{
		SOCKET socket;
		addrinfo* addressInfo;
		std::string url;
	} socketData;

	struct
	{
		std::thread* thread;
		std::atomic<StatusThread> status;
		std::mutex accesRequestBuf;
	} threadData;

	struct BufNode
	{
		int actualSize;
		char buf[SOCKET_BUF_SIZE];
	};

	std::deque<ArrayChar> requestBuf;

	static void Thread(ThreadHttp* ts);

	bool OpenSocket();
	void CloseSocket();
	void ReadSocket();

	void SendRequest();

	void PushRequest(ThreadHttp::ArrayChar &buf);
};

#endif
