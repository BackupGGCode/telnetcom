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

boost::random::mt19937 rng;
int32 irand(int32 min, int32 max)
{
    boost::random::uniform_int_distribution<int32> r(min, max);
    return (int32) r(rng);
}

uint32 urand(uint32 min, uint32 max)
{
    boost::random::uniform_int_distribution<uint32> r(min, max);
    return (uint32) r(rng);
}

float32 frand(float32 min, float32 max)
{
    boost::random::uniform_real_distribution<float> r(min, max);
    return (float32) r(rng);
}