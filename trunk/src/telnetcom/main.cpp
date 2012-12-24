#include "telnetcomPCH.hpp"
#include "tcpServer.hpp"
#include "serialPort.hpp"
#include "cliManager.hpp"

void LoadConfigs()
{
    sConfig->CheckConfigFile();
    sServerGlobals->serverOptions.port1 = sConfig->GetStringDefault("Port1", "23");
    sServerGlobals->serverOptions.port2 = sConfig->GetStringDefault("Port2", "none");
    sServerGlobals->serverOptions.port3 = sConfig->GetStringDefault("Port3", "none");

    sSerialPort->serialOptions.device = sConfig->GetStringDefault("SerialPort", "none");
    sSerialPort->serialOptions.baudrate = sConfig->GetIntDefault("SerialBauds", 9600);

    sServerGlobals->AdminPass = sConfig->GetStringDefault("AdminPass", "admin");
    sServerGlobals->ModPass = sConfig->GetStringDefault("ModPass", "moderator");
}

void StartServer(boost::asio::io_service& io_service)
{
    try
    {
        const char* portsarray[] = {
            sServerGlobals->serverOptions.port1.c_str(), sServerGlobals->serverOptions.port2.c_str(), sServerGlobals->serverOptions.port3.c_str(), 0
        };

        chat_server_list servers;
        uint32 count = 0;
        for (int32 i = 0; portsarray[i] != 0; ++i)
        {
            if (strcmp(portsarray[i], "none") == 0)
                continue;

            boost::asio::ip::tcp::endpoint endpoint(boost::asio::ip::tcp::v4(), atoi(portsarray[i]));
            chat_server_ptr server(new chat_server(io_service, endpoint));
            servers.push_back(server);
            ++count;
        }
        if (count > 1)
            sServerGlobals->InMultipleMode = true;

        boost::thread thread_message(&UpdateAssynMessagesThread);

        io_service.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

void CheckVersion()
{
    try
    {
        std::stringstream spyout;
        std::vector<std::string> headers;
        int respuesta = GetHtmlWeb("servertest.sytes.net", "80", "/TelnetComV.htm", spyout, headers, 1000);

        // 200 es correcto, cualquier otra cosa es que hubo algun error, ej: 404 web not found
        if (respuesta != 200)
            return;

        float version = boost::lexical_cast<float>(spyout.str().c_str());

        if (version < RELEASE_VERSION)
            std::cout << "Existe una version mas reciente, es recomendable actualizar." << std::endl;
        else if (version > RELEASE_VERSION)
            std::cout << "Tienes una version mas reciente que la ultima, compilaste tu mismo del repositorio?." << std::endl;
        else
            std::cout << "Version de TelnetCom: " << version << std::endl;
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}

void main()
{
    CheckVersion();

    LoadConfigs();

    boost::asio::io_service io_service;

    if (sSerialPort->serialOptions.device != "none")
        SetUpSerialSystem(io_service);

    cliRunnable* cliRunnable_ = new cliRunnable;
    boost::thread t(boost::bind(&cliRunnable::run, cliRunnable_));

    // No saldra de esta funcion hasta finalizar el programa. (bucle)
    StartServer(io_service);
}