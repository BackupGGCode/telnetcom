#include "telnetcomPCH.hpp"
#include "tcpServer.hpp"
#include "cliManager.hpp"
#include "serialPort.hpp"

// TODO: Comandos no case-sensitive.

void CommandsMgr::LoadCommands()
{
    ListCommands lista[MAX_COMMANDS] =
    {
        {"config",          SEC_USER,      true,     &GetConfigOptionsHandler,     ""},
        {"shutdown",        SEC_ADMIN,     true,     &ShutdownCommandHandler,      ""},
        {"getip",           SEC_USER,      false,    &GetIpCommandHandler,         ""},
        {"getclientlist",   SEC_USER,      true,     &GetClientList,               ""},
        {"sendall",         SEC_USER,      true,     &SendAllHandler,              "Sintaxis: sendall [mensaje]"},
        {"send",            SEC_USER,      true,     &SendHandler,                 "Sintaxis: send [IP] [mensaje], cambie IP por Console, para un mensaje a la consola."},
        {"sendserial",      SEC_USER,      true,     &SendSerialHandler,           "Sintaxis: sendserial [Dato], solo puede ser un caracter."},
        {"kickall",         SEC_ADMIN,     true,     &KickAllClientsHandler,       ""},
        {"kick",            SEC_MODERATOR, true,     &KickClientHandler,           "Sintaxis: kick [IP]"},
        {"time",            SEC_USER,      true,     &timeServerHandler,           ""},
        {"login",           SEC_USER,      false,    &LoginClientHandler,          "Sintaxis: login [contraseña]"},
        {"unlogin",         SEC_USER,      false,    &UnLoginClientHandler,        ""},
        {"commands",        SEC_USER,      true,     &ComandsListHandler,          ""},
        {"help",            SEC_USER,      true,     &ComandsListHandler,          ""},
        {"END",             SEC_USER,      false,    NULL,                      "END"} // Fin del array, no modificar
    };

    for (int32 i = 0; lista[i].Name != "END"; ++i)
        MyComands_[i] = lista[i];
}

bool GetConfigOptionsHandler(chat_session_ptr _client, char const* args)
{
    CommandsMgr::PSendMessageBoth(_client, "Puerto 1 de tcp: %s", sServerGlobals->serverOptions.port1.c_str());
    CommandsMgr::PSendMessageBoth(_client, "Puerto 2 de tcp: %s", sServerGlobals->serverOptions.port2.c_str());
    CommandsMgr::PSendMessageBoth(_client, "Puerto 3 de tcp: %s", sServerGlobals->serverOptions.port3.c_str());
    CommandsMgr::PSendMessageBoth(_client, "\n");
    CommandsMgr::PSendMessageBoth(_client, "Puerto serial: %s", sSerialPort->serialOptions.device.c_str());
    CommandsMgr::PSendMessageBoth(_client, "Velocidad del puerto serial: %u", sSerialPort->serialOptions.baudrate);

    return true;
}

bool ShutdownCommandHandler(chat_session_ptr _client, char const* args)
{
    if (sSerialPort->Set_ && sSerialPort->mSerialPort->active())
        sSerialPort->mSerialPort->close();

    if (_client)
    {
        sServerGlobals->SendMessageAllClients("El servidor se desconectara en 5 segundos.");
        boost::asio::deadline_timer t(_client->io_service_, boost::posix_time::seconds(5));
        t.wait();

        _client->io_service_.stop();
    }
    else
        exit(EXIT_SUCCESS);

    return true;
}

bool GetIpCommandHandler(chat_session_ptr _client, char const* args)
{
    std::string s = "Tu IP es: " + _client->socket().remote_endpoint().address().to_string();
    _client->SendMessageClient(s);

    return true;
}

bool GetClientList(chat_session_ptr _client, char const* args)
{
    std::list<chat_session_ptr>::const_iterator itr = sServerGlobals->GetClientList().begin();

    for (; itr != sServerGlobals->GetClientList().end(); ++itr)
    {
        if ((*itr) && (*itr)->socket().is_open())
        {
            std::string s = "-- " + (*itr)->socket().remote_endpoint().address().to_string();
            if ((*itr)->GetSecurity() > SEC_USER)
            {
                s += " --- ";
                s += GetSecurityString((*itr)->GetSecurity());
            }
            CommandsMgr::PSendMessageBoth(_client, s.c_str());
        }
    }
    CommandsMgr::PSendMessageBoth(_client, "Hay %u clientes conectados actualmente.", uint32(sServerGlobals->GetClientList().size()));

    return true;
}

bool SendAllHandler(chat_session_ptr _client, char const* args)
{
    if (!args)
        return false;

    std::string from = std::string();
    if (!_client)
        from = "Console: ";
    else
        from = _client->socket().remote_endpoint().address().to_string() + ": ";

    from += args;

    sServerGlobals->AssynSendMessageAllClients(from);

    return true;
}

bool SendHandler(chat_session_ptr _client, char const* args)
{
    if (!args)
        return false;

    char* to_ = strtok((char*)args, " ");

    char* msg_ = strtok(NULL, "");
    if (!to_ || !msg_)
        return false;

    std::string from = std::string();
    if (!_client)
        from = "Console: ";
    else
        from = _client->socket().remote_endpoint().address().to_string() + ": ";

    from += msg_;

    if (strcmp(to_, "Console") == 0)
    {
        std::cout << from;
        return true;
    }

    std::list<chat_session_ptr>::const_iterator itr = sServerGlobals->GetClientList().begin();
    bool send_ = false;
    for (; itr != sServerGlobals->GetClientList().end(); ++itr)
    {
        if ((*itr) && (*itr)->socket().is_open())
        {
            if ((*itr)->socket().remote_endpoint().address().to_string() == to_)
            {
                (*itr)->SendMessageClient(from);
                send_ = true;
            }
        }
    }

    if (!send_)
        CommandsMgr::SendMessageBoth(_client, "No se a encontrado un cliente con esa IP.");

    return true;
}

bool SendSerialHandler(chat_session_ptr _client, char const* args)
{
    if (!args)
        return false;

    std::string mesg = std::string(args);
    // Borramos el espacio que tiene al principio.
    replace_string(mesg, " ", "");

    if (sSerialPort->Set_ && sSerialPort->mSerialPort->active())
        sSerialPort->SendSerialData(mesg);
    else
        CommandsMgr::SendMessageBoth(_client, "Esta desactivado o no esta activo el puerto serial");

    return true;
}

bool KickAllClientsHandler(chat_session_ptr _client, char const* args)
{
    uint32 count = 0;
    std::list<chat_session_ptr>::const_iterator itr = sServerGlobals->GetClientList().begin();
    for (; itr != sServerGlobals->GetClientList().end(); ++itr)
    {
        if ((*itr) && (*itr)->socket().is_open())
        {
            (*itr)->socket().close();
            ++count;
        }
    }

    CommandsMgr::PSendMessageBoth(_client, "Se han kikeado %u, clientes.", count);
    return true;
}

bool KickClientHandler(chat_session_ptr _client, char const* args)
{
    if (!args)
        return false;

    char* ip_str = strtok((char*)args, " ");

    if (!ip_str)
        return false;

    std::list<chat_session_ptr>::const_iterator itr = sServerGlobals->GetClientList().begin();

    for (; itr != sServerGlobals->GetClientList().end(); ++itr)
    {
        if ((*itr) && (*itr)->socket().is_open())
            if ((*itr)->socket().remote_endpoint().address().to_string() == ip_str)
            {
                (*itr)->socket().close();
                CommandsMgr::PSendMessageBoth(_client, "Se ha kikeado al cliente: %s.", ip_str);
                return true;
            }
    }

    CommandsMgr::PSendMessageBoth(_client, "No se encontro el cliente: %s.", ip_str);
    return true;
}

bool timeServerHandler(chat_session_ptr _client, char const* args)
{
    time_t start_time = sServerGlobals->startTime;
    time_t now_time = time(NULL);

    uint32 segundos = uint32 (now_time - start_time);
    float32 minutos = (float32) segundos / 60;
    float32 horas = (float32) minutos / 60;
    float32 dias = (float32) horas / 24;

    CommandsMgr::PSendMessageBoth(_client, "TelnetCom inicio: %s", ctime(&start_time));
    CommandsMgr::PSendMessageBoth(_client, "El tiempo actual es: %s", ctime(&now_time));
    CommandsMgr::PSendMessageBoth(_client, "Han transcurrido, %f dias, %f horas, %f minutos, %u segundos.", dias, horas, minutos, segundos);

    return true;
}

bool ComandsListHandler(chat_session_ptr _client, char const* args)
{
    ListCommands* commands_ = sCommandsMgr->GetCommandsList();

    CommandsMgr::SendMessageBoth(_client, "Lista de comandos disponibles: ");

    for (int32 i = 0; commands_[i].Name != "NONE"; ++i)
    {
        if (!_client && !commands_[i].AllowConsole)
            continue;

        if (_client && _client->GetSecurity() < commands_[i].sec)
            continue;

        CommandsMgr::PSendMessageBoth(_client, "%s", commands_[i].Name.c_str());
    }

    return true;
}

bool LoginClientHandler(chat_session_ptr _client, char const* args)
{
    // El !_client no deberia pasar JAMAS.
    if (!args || !_client)
        return false;

    char* pass_ = strtok((char*)args, " ");

    if (!pass_)
        return false;

    if (pass_ == sServerGlobals->AdminPass)
        _client->sec_level = SEC_ADMIN;
    else if (pass_ == sServerGlobals->ModPass)
        _client->sec_level = SEC_MODERATOR;
    else
    {
        CommandsMgr::SendMessageBoth(_client, "Contraseña incorrecta.");
        return true;
    }

    CommandsMgr::PSendMessageBoth(_client, "Se ha logeado con permisos de %s.", GetSecurityString(_client->GetSecurity()).c_str());
    return true;
}

bool UnLoginClientHandler(chat_session_ptr _client, char const* args)
{
    // El !_client no deberia pasar JAMAS.
    if (!_client)
        return false;

    _client->sec_level = SEC_USER;

    CommandsMgr::SendMessageBoth(_client, "Se ha deslogeado con exito.");
    return true;
}

// Inicio del sistema de comandos y consola.

void cliRunnable::run()
{
    std::cout << "TC> ";
    while (true)
    {
        char command_str[500];
        memset(command_str, 0, sizeof (command_str));
        std::cin.getline(command_str, 500, '\n');
        if (command_str != NULL)
        {
            for (int32 x=0; command_str[x]; ++x)
                if (command_str[x] == '\r' || command_str[x] == '\n')
                {
                    command_str[x] = 0;
                    break;
                }
                if (!*command_str)
                {
                    std::cout << "TC> ";
                    continue;
                }

                std::string command = command_str;

                // Declaramos un chat_session_ptr para el argumento, con suerte esto sera NULL, de otro modo crasheara.
                chat_session_ptr nullSession;
                sCommandsMgr->QueueCommand(nullSession, command, true);
        }
    }
}

void CommandsMgr::QueueCommand(chat_session_ptr _client, std::string command, bool console)
{
    CHandler handler = {_client, console};

    std::pair<std::string, CHandler> com_pair(command, handler);
    commandQueue.insert(com_pair); // Thread un-safe?

    if (!InProsses)
        boost::thread t(boost::bind(&CommandsMgr::CommandQueueProssesor, this));
}

void CommandsMgr::CommandQueueProssesor()
{
    InProsses = true;

    for (std::map<std::string, CHandler>::iterator itr = commandQueue.begin(); itr != commandQueue.end(); ++itr)
        RunCommand(itr->second.client, itr->first, itr->second.console);

    commandQueue.clear();

    InProsses = false;
}

void CommandsMgr::RunCommand(chat_session_ptr _client, std::string command, bool console)
{
    const char* command_str = _strdup(command.c_str());
    char* tok = strtok((char*) command_str, " ");
    tok = strtok(NULL, " ");
    std::ostringstream args;
    while (tok != NULL)
    {
        args << " " << tok;
        tok = strtok(NULL, " ");
    }
    char* arg_f = (char*)args.str().c_str();

    memcpy ( arg_f, args.str().c_str(), strlen(args.str().c_str()) +1);
    if (strcmp(arg_f, "") == 0)
        arg_f = NULL;


    for (int32 i = 0; i < MAX_COMMANDS; ++i)
    {
        if (command_str == MyComands_[i].Name)
        {
            if (!MyComands_[i].AllowConsole && console)
            {
                std::cout << "Este comando no se puede usar por consola." << std::endl;
                break;
            }

            if (!console && _client->GetSecurity() < MyComands_[i].sec)
            {
                CommandsMgr::SendMessageBoth(_client, "No tienes permisos para ejecutar este comando!");
                CommandsMgr::PSendMessageBoth(_client, "Este comando requiere permisos de: %s.", GetSecurityString(MyComands_[i].sec).c_str());
                break;
            }

            if (!MyComands_[i].handler(_client, arg_f))
            {
                CommandsMgr::SendMessageBoth(_client, "Error de sintaxis.");
                CommandsMgr::PSendMessageBoth(_client, "%s", MyComands_[i].Help.c_str());
            }

            break;
        }
    }

    // Esto manda al cliente el salto de linea e incluye TC>
    if (_client)
        _client->SendMessageClient("");

    std::cout << std::endl << "TC> ";
    command.clear();
    args.clear();
    free(tok);
}

bool CommandsMgr::SendMessageBoth(chat_session_ptr _client, char const* msg)
{
    if (!_client)
    {
        std::cout << msg << std::endl;
        return true;
    }

    std::string msg_ = msg;

    if (!_client->SendMessageClient(msg_))
        return false;

    return true;
}

bool CommandsMgr::PSendMessageBoth(chat_session_ptr _client, char const* format, ...)
{
    char str [2048];
    va_list ap;
    va_start(ap, format);
    vsnprintf(str, 2048, format, ap);
    va_end(ap);

    if (!SendMessageBoth(_client, str))
        return false;

    return true;
}

void CommandsMgr::SendMessageConsole(char const* msg)
{
    std::cout << msg << std::endl;
}

void CommandsMgr::PSendMessageConsole(char const* format, ...)
{
    va_list ap;
    char str [2048];
    va_start(ap, format);
    vsnprintf(str, 2048, format, ap);
    va_end(ap);
    SendMessageConsole(str);
}

std::string GetSecurityString(SecurityTypes sec)
{
    std::string sec_string =  std::string();
    switch (sec)
    {
    case SEC_USER:
        sec_string = "Usuario";
        break;
    case SEC_MODERATOR:
        sec_string = "Moderador";
        break;
    case SEC_ADMIN:
        sec_string = "Administrador";
        break;
    default:
        sec_string = "Desconocido";
        break;
    }

    return sec_string;
}