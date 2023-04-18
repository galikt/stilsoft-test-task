/*
Тестовое задание на должность программиста ООО «Стилсофт»
Владимир Герасимчук
sugalikt@gmail.com
*/

#include <iostream>
#include <locale.h>
#include <conio.h>
#include "thread_http.h"

const int ESC{ 27 };

int main()
{
	setlocale(LC_ALL, "ru_RU.UTF-8");

	std::string url;
	std::cout << "Enter site url: ";
	std::cin >> url;

	ThreadHttp* http{ new ThreadHttp(url) };

	char* buf{ nullptr };
	for (;;)
	{
		if (_kbhit() && (_getch() == ESC))
			break;

		ThreadHttp::ArrayChar buf;
		while (buf = http->PopRequest())
		{
			std::cout << buf;
		}

		Sleep(1000);
	}

	delete http;

	return 0;
}