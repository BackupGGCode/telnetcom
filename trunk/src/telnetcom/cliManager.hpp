#pragma once

#define MAX_COMMANDS 100


// Declaracion de los handlers de los comandos

bool GetConfigOptionsHandler(chat_session_ptr _client, const char* args);
bool ShutdownCommandHandler(chat_session_ptr _client, const char* args);
bool GetIpCommandHandler(chat_session_ptr _client, const char* args);
bool GetClientList(chat_session_ptr _client, const char* args);
bool SendAllHandler(chat_session_ptr _client, const char* args);
bool SendHandler(chat_session_ptr _client, const char* args);
bool SendSerialHandler(chat_session_ptr _client, const char* args);
bool KickAllClientsHandler(chat_session_ptr _client, const char* args);
bool KickClientHandler(chat_session_ptr _client, const char* args);
bool timeServerHandler(chat_session_ptr _client, const char* args);
bool ComandsListHandler(chat_session_ptr _client, const char* args);
bool LoginClientHandler(chat_session_ptr _client, const char* args);
bool UnLoginClientHandler(chat_session_ptr _client, const char* args);
bool ToggleReadSerialHandler(chat_session_ptr _client, char const* args);

// Declaracion de utilidades.
std::string GetSecurityString(SecurityTypes sec);

// Clases de comandos y consola.

class cliRunnable
{
public:
    void run();
};
class CommandsMgr;

struct ListCommands
{
    std::string Name;
    SecurityTypes sec;
    bool AllowConsole;
    bool (*handler)(chat_session_ptr, const char* args);
    std::string Help;
};

struct CHandler
{
    chat_session_ptr client;
    bool console;
};

class CommandsMgr
{
public:
    CommandsMgr()
    {
        for (int32 i = 0; i < MAX_COMMANDS; ++i)
        {
            MyComands_[i].Name = "NONE";
            MyComands_[i].handler = NULL;;
            MyComands_[i].Help = "NONE";
        }

        LoadCommands();

        InProsses = false;
    }

    void QueueCommand(chat_session_ptr _client, std::string command, bool console = false);
    void RunCommand(chat_session_ptr _client, std::string command, bool console = false);

    static bool SendMessageBoth(chat_session_ptr _client, const char* msg);
    static bool PSendMessageBoth(chat_session_ptr _client, const char* format, ...);
    static void SendMessageConsole(const char* msg);
    static void PSendMessageConsole(const char* format, ...);
    ListCommands* const GetCommandsList() { return MyComands_; }


private:

    void CommandQueueProssesor();
    void LoadCommands();

    ListCommands MyComands_[MAX_COMMANDS];
    bool InProsses;
    std::map<std::string, CHandler> commandQueue;
};

#define sCommandsMgr CSingletonT<CommandsMgr>::Get()