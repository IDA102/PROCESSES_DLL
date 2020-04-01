#include "processes_dll.h"

int main()
{
	bool cmp = 0;
	//out_test();
	std::vector<SERVICE> q = get_BD_processes();
	cmp = START_SERVICE(q[0].NAME);
	cmp = STOP_SERVICE(q[0].NAME);
	int rez = RESTART_SERVICE(q[0].NAME);
 	return 0;
}