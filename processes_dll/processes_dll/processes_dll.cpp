#pragma once
#include "pch.h" 
#include <utility>
#include <limits.h>
#include "processes_dll.h"
#define _CRT_SECURE_NO_WARNINGS
#define	  stop __asm nop
#include <Windows.h>
#include <iostream>
#include <stdexcept>
#include <string>
#include <vector>
#include "atlbase.h"
#include "atlstr.h"
#include "comutil.h"

bool out_test()
{
	std::cout << "START TEST" << std::endl;
	std::cout << 1 << " " << 2 << " " << 3 << " " << 4 << std::endl;
	std::cout << "END TEST" << std::endl;
	return true;
}
//Метод получения копии базы данных служб диспетчера задач
std::vector<SERVICE> get_BD_processes()
{
	SC_HANDLE discriptor = nullptr, discriptor_service = nullptr;//Дескриптор служб локальной машины.
	bool err_discriptor, err_config_service,stoped;//флаг ошибки
	ENUM_SERVICE_STATUS_PROCESS *SERVICE_BUFER;//Указатель на буфер в котором содержатся стркуктуры "всех" служб
	QUERY_SERVICE_CONFIG *SERVICE_CONFIG_BUFER;//Указатель на буфер в котором содержится стркуктура выбранной службы
	DWORD dwBytesNeeded = 0;//Сколько тркбуется памяти в байтах для дескриптора всех процессов
	DWORD dwBytesNeeded2 = 0;//Сколько тркбуется памяти в байтах для дескриптора одного конткретного процесса
	DWORD dwServicesReturned = 0;//Количество возвращённых служб
	DWORD dwResumeHandle = 0;

	//Открываем дескриптор процессов
	try
	{
		if (discriptor = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS))
		{
			//Выделяем память под размер дескрипьтора (размер вернёт dwBytesNeeded)
			err_discriptor = EnumServicesStatusEx(
				discriptor,
				SC_ENUM_PROCESS_INFO,
				SERVICE_WIN32,//SERVICE_DRIVER
				SERVICE_STATE_ALL,
				NULL,
				0,
				&dwBytesNeeded,
				&dwServicesReturned,
				NULL,
				NULL
			);
			//Выделяем память и делаем копию дескриптора служб
			DWORD dwBytes = dwBytesNeeded + sizeof(ENUM_SERVICE_STATUS_PROCESS);
			SERVICE_BUFER = (ENUM_SERVICE_STATUS_PROCESS*)LocalAlloc(LPTR, dwBytes);
			if (SERVICE_BUFER == NULL) { throw("not create memory SERVICE_BUFFER (malloc)"); }

			err_discriptor = EnumServicesStatusEx
			(
				discriptor,
				SC_ENUM_PROCESS_INFO,
				SERVICE_WIN32,//SERVICE_DRIVER
				SERVICE_STATE_ALL,
				(LPBYTE)SERVICE_BUFER,
				dwBytes,
				&dwBytesNeeded,
				&dwServicesReturned,
				&dwResumeHandle,
				NULL
			);

			//Выделяем память для структур служб передаваемых в с++++
			std::vector<SERVICE> SERVICE_VECTOR((size_t)dwServicesReturned);
			for (size_t i = 0; i < (size_t)dwServicesReturned; ++i)
			{

				//Открываем службу
				try
				{
					discriptor_service = OpenService(discriptor, SERVICE_BUFER[i].lpServiceName, SERVICE_ALL_ACCESS);
				}
				catch (const std::exception)
				{
					std::cout << "Read memory error in process descriptor--------cath(std::exception)" << std::endl;
				}
				catch (...)
				{
					std::cerr << "Read memory error in process descriptor!--------cath(...)\n";
					stop
				}

				err_config_service = QueryServiceConfig
				(
					discriptor_service,
					NULL,
					NULL,
					&dwBytesNeeded2
				);

				//Вытаскиваем путь и группу
				DWORD dwBytes2 = dwBytesNeeded2 + sizeof(SERVICE_CONFIG_BUFER);
				SERVICE_CONFIG_BUFER = (QUERY_SERVICE_CONFIG*)LocalAlloc(LPTR, dwBytes2);
				if (SERVICE_CONFIG_BUFER == NULL) { throw("not create memory SERVICE_CONFIG_BUFER (malloc)"); }

				err_config_service = QueryServiceConfig
				(
					discriptor_service,
					SERVICE_CONFIG_BUFER,
					dwBytes2,
					&dwBytesNeeded2
				);
				//Заполняем элемент вектора структурой
				SERVICE_VECTOR[i].NAME = SERVICE_BUFER[i].lpServiceName;
				SERVICE_VECTOR[i].PID = SERVICE_BUFER[i].ServiceStatusProcess.dwProcessId;
				SERVICE_VECTOR[i].DECRIPTION = SERVICE_BUFER[i].lpDisplayName;
				SERVICE_VECTOR[i].STATUS = (status)SERVICE_BUFER[i].ServiceStatusProcess.dwServiceFlags;
				try
				{		
					if (SERVICE_CONFIG_BUFER->lpBinaryPathName != NULL)
					{
						std::wstring cmp = SERVICE_CONFIG_BUFER->lpBinaryPathName;
						int  o = cmp.find(L" -k ");
						if (o != -1)
						{
							SERVICE_VECTOR[i].GROUP = &cmp[o + 4];
							cmp[o] = '\0';
							SERVICE_VECTOR[i].IMAGE_PATH = cmp;
						}
						else SERVICE_VECTOR[i].IMAGE_PATH = cmp;
					}
				}
				catch (...)
				{
					std::cerr << "Read memory error in process descriptor in service --- " << i << std::endl;
				}
				try
				{
					if (discriptor_service != NULL)
					{
						stoped = CloseServiceHandle(discriptor_service);
						if (!stoped)
						{
							LocalFree(SERVICE_CONFIG_BUFER);
							throw("not close  descriptor service");
						}
					}
				}
				catch (const char* err)	{ std::cout << err << std::endl;}
				LocalFree(SERVICE_CONFIG_BUFER);
			}	
			try
			{
				stoped = CloseServiceHandle(discriptor);
				if (!stoped)
				{
					LocalFree(SERVICE_BUFER);
					throw("not close  descriptor BD");
				}
			}
			catch (const char* err) { std::cout << err << std::endl; }
			LocalFree(SERVICE_BUFER);

			stop
			return SERVICE_VECTOR;//<------------------------------------------------------------------------------------------------------------------
		}
		else { throw("not creat descriptor"); }
	}
	catch (const char* err)
	{
		std::cout << err << std::endl;
		exit(0);
	}
}

//Функция запуска службы 
bool START_SERVICE(std::wstring lpServiceName)
{
	SC_HANDLE discriptor = nullptr, discriptor_service = nullptr;//Дескриптор служб локальной машины.
	BOOL ret = 0;
	DWORD GLE = 1056;//Такую ошибку возращает "OpenService", если служба запущена.//1056
	//Открываем дескриптор процессов
	try
	{
		if (!(discriptor = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)))
		{ throw("not creat descriptor in start service");}
	}
	catch (const char* err)
	{
		std::cout << err << std::endl;
		return 0;
	}
	try
	{
		if(!(discriptor_service = OpenService(discriptor, &lpServiceName[0], SERVICE_ALL_ACCESS)))
		{ throw("not creat descriptor in open service"); }
	}
	catch (const char* err)
	{
		std::cout << err << std::endl;
		return 0;
	}

//	while (GLE == 1056)
//	{
	ret = StartService(discriptor_service, NULL, NULL);//3
//		if (!ret)
//		{
//			GLE = GetLastError();
//		}
//		else return ret;
//	}
	bool stoped = CloseServiceHandle(discriptor_service);
	if (!stoped)
	{
		CloseServiceHandle(discriptor);
		return false;
	}
	stoped = CloseServiceHandle(discriptor);
	if (!stoped) { return false; }
	return ret;
	/*
	Здесь не нужно ожидать ответа от функции или обновления службы как в "ControlService" f STOP_SERVICE, т.к. функция сама ожидает гарантированного запуска службы
	и единственная ошибка которая может произойти, это ошибка времени запуска(ждёт 20 сек + можно опрашивать состояние, запустилась или нет).
	Цититрую "Когда запускается сервисный драйвер, функция StartService не возвращает значение до тех пор, пока драйвер устройства не  закончит инициализацию."
	*/
}

//Функция остановки службы 
bool STOP_SERVICE(std::wstring lpServiceName)
{
	SERVICE_STATUS status;
	bool stoped = false;
	SC_HANDLE discriptor = nullptr, discriptor_service = nullptr;//Дескриптор служб локальной машины.
	//Открываем дескриптор процессов
	try
	{
		if (!(discriptor = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS)))
		{
			throw("not creat descriptor in start service");
		}
	}
	catch (const char* err)
	{
		std::cout << err << std::endl;
		return 0;
	}
	try
	{
		if (!(discriptor_service = OpenService(discriptor, &lpServiceName[0], SERVICE_ALL_ACCESS)))
		{
			throw("not creat descriptor in open service");
		}
	}
	catch (const char* err)
	{
		std::cout << err << std::endl;
		return 0;
	}

	
	BOOL ret = ControlService(discriptor_service, SERVICE_CONTROL_STOP, &status);
	if (!ret) 
	{ 
		DWORD GLE = GetLastError();
		if ((GLE == 3435973836) || (GLE == 1062))	/*3435973836 и 1062<--- это значения возможных ошибок, если произошла попытка остановить уже остановленную службу*/
		{
			return true;
		}
		else
		{
			//if (GetLastError() == ERROR_SERVICE_REQUEST_TIMEOUT);//Можно написать исключение о таймауте или о невозможности закрытия}
			return 0;
		}
	}

	while (status.dwCurrentState != 1)	{ QueryServiceStatus(discriptor_service, &status); }

 	stoped = CloseServiceHandle(discriptor_service);
	if (!stoped)
	{
		CloseServiceHandle(discriptor);
		return false;
	}
	stoped = CloseServiceHandle(discriptor);
	if (!stoped){return false;}
	return true;
}

//Функция перезапуска службы
int RESTART_SERVICE(std::wstring lpServiceName)
{
	int k = 0;
	if (!STOP_SERVICE(lpServiceName)) { k = 2; return k; }
	if (!START_SERVICE(lpServiceName)) { k = 1; return k; }
	return k;
}