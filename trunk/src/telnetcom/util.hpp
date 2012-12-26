#pragma once

#define RELEASE_VERSION 1.0f

typedef boost::mutex try_mutex;

/* ¡Definicion de typos estandar! */
typedef signed char int8;
typedef unsigned char uint8;
typedef short int16;
typedef unsigned short uint16;
typedef int int32;
typedef unsigned int uint32;

#ifdef _MSC_EXTENSIONS
typedef __int64 int64;
typedef unsigned __int64 uint64;
#elif ! defined(_MSC_VER)
typedef int64_t int64;
typedef uint64_t uint64;
#else
typedef long long int64;
typedef unsigned long long uint64;
#endif

typedef float float32;
typedef double float64;

#define NLINE "\n\rTC> "

enum SecurityTypes
{
    SEC_USER           = 0,
    SEC_MODERATOR      = 1,
    SEC_ADMIN          = 2
};

void delay(int32 secs);
void wait (int32 ms);
bool replace_string(std::string& str, const std::string& from, const std::string& to);

int32 irand(int32 min, int32 max);
uint32 urand(uint32 min, uint32 max);
float32 frand(float32 min, float32 max);

int GetHtmlWeb(std::string host_,std::string port_, std::string url_path,std::stringstream &out_,std::vector<std::string> &headers, unsigned int timeout);

/*
#### Implementacion de Singleton Thread-Safe.
*/
template <typename T>
class CSingletonT
{
private:
    CSingletonT();
    ~CSingletonT();
    const CSingletonT& operator=(const CSingletonT& src);

protected:
    static volatile LONG m_lLocker;

    static T* GetWkr()
    {
        T* pTmp = NULL;
        try
        {
            static T tVar;
            pTmp = &tVar;
        }
        catch(...)
        {
            _ASSERT(FALSE);
            pTmp = NULL;
        }
        return pTmp;
    }

public:
    static T* Get()
    {
        while (::InterlockedExchange(&m_lLocker, 1) != 0)
        {
            Sleep(1);
        }
        T* pTmp = GetWkr();
        ::InterlockedExchange(&m_lLocker, 0);
        return pTmp;
    }
};

template <typename T>
volatile LONG CSingletonT<T>::m_lLocker = 0;