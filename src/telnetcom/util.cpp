#include "telnetcomPCH.hpp"
#include "tcpServer.hpp"
#include "util.hpp"

void delay(int32 secs)
{
	for(int32 i = int32(time(NULL) + secs); time(NULL) != i; time(NULL));
}

void wait (int32 ms)
{
	clock_t endwait;
	endwait = clock () + ms;
	while (clock() < endwait) {}
}

bool replace_string(std::string& str, const std::string& from, const std::string& to) 
{
	size_t start_pos = str.find(from);
	if(start_pos == std::string::npos)
		return false;
	str.replace(start_pos, from.length(), to);
	return true;
}