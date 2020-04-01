#pragma once
#include <vector>
enum status { Running, Stopping, Paused };
enum management_teams { Start, Stop, Restart };

struct SERVICE
{
	std::wstring NAME;
	std::wstring GROUP;
	std::wstring DECRIPTION;
	std::wstring IMAGE_PATH;
	size_t PID = 0;
	status STATUS;
};

#ifdef MATHLIBRARY_EXPORTS
#define MATHLIBRARY_API __declspec(dllexport)
#else
#define MATHLIBRARY_API __declspec(dllimport)
#endif

extern "C++" MATHLIBRARY_API bool out_test();
extern "C++" MATHLIBRARY_API std::vector<SERVICE> get_BD_processes();
extern "C++" MATHLIBRARY_API bool START_SERVICE(std::wstring lpServiceName);
extern "C++" MATHLIBRARY_API bool STOP_SERVICE(std::wstring lpServiceName);
extern "C++" MATHLIBRARY_API int  RESTART_SERVICE(std::wstring lpServiceName);