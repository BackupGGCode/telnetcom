#pragma once


// ServerMessage Update thread
void UpdateAssynMessagesThread();

class chat_session : public boost::enable_shared_from_this<chat_session>
{
public:
    chat_session(boost::asio::io_service& io_service): socket_(io_service), io_service_(io_service)
    {
        memset(buffer_, 0, sizeof (buffer_));
        sec_level = SEC_USER;
        ReadSerialData = false;
    }

    boost::asio::ip::tcp::socket& socket()
    {
        return socket_;
    }

    bool SendMessageClient(std::string _message)
    {
        if (!socket_.is_open())
            return false;

        _message += NLINE;

        boost::system::error_code error;
        socket_.write_some(boost::asio::buffer(_message), error);

        if (error)
            return false;
        else
            return true;
    }

    void start();
    void ReadClientThread();

    bool ReadSerialData;

    SecurityTypes GetSecurity() { return sec_level; };
    SecurityTypes sec_level;

    boost::asio::io_service& io_service_;

private:
    boost::asio::ip::tcp::socket socket_;
    char buffer_[100];

};

typedef boost::shared_ptr<chat_session> chat_session_ptr;
//----------------------------------------------------------------------

class chat_server
{
public:
    chat_server(boost::asio::io_service& io_service, const boost::asio::ip::tcp::endpoint& endpoint) : io_service_(io_service), acceptor_(io_service, endpoint)
    {
        start_accept();
        std::cout << "TCP> Escuchando en el puerto: " << acceptor_.local_endpoint().port() << "." << std::endl;
    }

    void start_accept();

    void handle_accept(chat_session_ptr session, const boost::system::error_code& error);

private:
    boost::asio::io_service& io_service_;
    boost::asio::ip::tcp::acceptor acceptor_;
};

typedef boost::shared_ptr<chat_server> chat_server_ptr;
typedef std::list<chat_server_ptr> chat_server_list;


//----------------------------------------------------------------------

// Estructura para almacenar las opciones.
struct ServerOptions
{
    std::string port1;
    std::string port2;
    std::string port3;
};

class ServerGlobals
{
public:
    ServerGlobals() 
    { 
        // Los constructores de los singletons, son llamados al momento de iniciar el programa.
        startTime = time(NULL);
        InMultipleMode = false;
        _ClientList.clear();
        _AssynMessageList.clear();
    }

    void SendMessageAllClients(std::string _message)
    {
        boost::system::error_code error;
        AcquireLock();
        _message += NLINE;
        std::cout << _message;
        for (std::list<chat_session_ptr>::iterator iter = _ClientList.begin(); iter != _ClientList.end(); ++iter)
        {
            (*iter)->socket().write_some(boost::asio::buffer(_message), error);
        }
        ReleaseLock();
    }

    void AssynSendMessageAllClients(std::string _message)
    {
        _message += NLINE;
        std::cout << _message;
        boost::thread thread_message(boost::bind(&ServerGlobals::FinalAddAMessage, this, _message));
    }

    // Thread para agregar el mensaje en la lista, de otro modo los locks pueden causar retraso en la llamada.
    void FinalAddAMessage(std::string _message)
    {
        AcquireLock();
        _AssynMessageList.push_back(_message);
        ReleaseLock();
    }

    // Actualizamos los mensajes assyncronos.
    void Update()
    {
        if (_AssynMessageList.empty())
            return;

        for (std::list<std::string>::iterator iter = _AssynMessageList.begin(); iter != _AssynMessageList.end(); ++iter)
        {
            for (std::list<chat_session_ptr>::iterator iter_c = _ClientList.begin(); iter_c != _ClientList.end(); ++iter_c)
            {
                (*iter_c)->socket().write_some(boost::asio::buffer(*iter));
            }
        }

        _AssynMessageList.clear();
    }

    void AddClient(chat_session_ptr _client)
    {
        AcquireLock();
        _ClientList.push_back(_client);
        ReleaseLock();
    }

    void RemoveClient(chat_session_ptr _client)
    {
        AcquireLock();
        _ClientList.remove(_client);
        ReleaseLock();
    }

    void AcquireLock() { mLock.lock(); }
    void ReleaseLock() { mLock.unlock(); }

    std::list<chat_session_ptr> const& GetClientList()
    {
        return _ClientList;
    }

    ServerOptions serverOptions;
    bool InMultipleMode;
    std::string AdminPass;
    std::string ModPass;
    time_t startTime;
private:
    boost::mutex mLock;
    std::list<chat_session_ptr> _ClientList;
    std::list<std::string> _AssynMessageList;
};

#define sServerGlobals CSingletonT<ServerGlobals>::Get()