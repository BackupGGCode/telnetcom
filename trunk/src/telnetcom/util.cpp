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

int GetHtmlWeb(std::string host_,std::string port_, std::string url_path,std::stringstream &out_,std::vector<std::string> &headers, unsigned int timeout)
{
    try
    {
        using namespace boost::asio::ip;
        tcp::iostream request_stream;

        if (timeout>0)
            request_stream.expires_from_now(boost::posix_time::milliseconds(timeout));

        request_stream.connect(host_,port_);
        if(!request_stream)
            return -1;

        request_stream << "GET " << url_path << " HTTP/1.0\r\n";
        request_stream << "Host: " << host_ << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "Cache-Control: no-cache\r\n";
        request_stream << "Connection: close\r\n\r\n";
        request_stream.flush();
        std::string line1;
        std::getline(request_stream,line1);

        if (!request_stream)
            return -2;

        std::stringstream response_stream(line1);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream,status_message);

        if (!response_stream||http_version.substr(0,5)!="HTTP/")
            return -1;

        if (status_code!=200)
            return (int)status_code;

        std::string header;
        while (std::getline(request_stream, header) && header != "\r")
            headers.push_back(header);
        out_ << request_stream.rdbuf();
        return status_code;
    }
    catch(std::exception &e)
    {
        std::cout << e.what() << std::endl;
        return -3;
    }
}
