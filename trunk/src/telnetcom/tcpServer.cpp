#include "telnetcomPCH.hpp"
#include "tcpServer.hpp"
#include "cliManager.hpp"

using boost::asio::ip::tcp;

void chat_session::start()
{
    // Iniciamos un nuevo thread que este leyendo.
    sServerGlobals->AddClient(shared_from_this());
    boost::thread thread_session(boost::bind(&chat_session::ReadClientThread, shared_from_this()));
}

void chat_session::ReadClientThread()
{
    boost::system::error_code error;

    std::string stream_buffer;
    // El mensaje es mas que nada para que el cliente remoto active la escritura.
    //sServerGlobals->AssynSendMessageAllClients("Se unio un cliente.");
    std::string welcome_msg = "Bienvenido!, tu ip es: ";
    welcome_msg += socket().remote_endpoint().address().to_string();

    SendMessageClient(welcome_msg);

    while (!error)
    {
        socket_.read_some(boost::asio::buffer(buffer_), error);

        // Control + C
        if (*buffer_ == 3)
        {
            socket_.close();
            sServerGlobals->RemoveClient(shared_from_this());
            return;
        }

        // Enter, o salto de linea.
        if (*buffer_ == 13)
        {
            //replace_string(stream_buffer.str(), old_stream_buffer, "");
            memset(buffer_, 0, sizeof (buffer_));

            std::cout << "Receive: " << stream_buffer << std::endl;

            //sServerGlobals->AssynSendMessageAllClients(stream_buffer);
            //sServerGlobals->AssynSendMessageAllClients("\n\rTC> ");
            sCommandsMgr->QueueCommand(shared_from_this(), stream_buffer, false);

            stream_buffer = ( std::string() );
            stream_buffer.clear();
        } else stream_buffer = stream_buffer + buffer_;

        // Limpiamos el buffer o de lo contrario queda con basura.
        memset(buffer_, 0, sizeof (buffer_));

        //SendMessage("\n\rTC> ");

        Sleep(10);
    }
    sServerGlobals->RemoveClient(shared_from_this());
}


void chat_server::start_accept()
{
    chat_session_ptr new_session(new chat_session(io_service_));
    acceptor_.async_accept(new_session->socket(), boost::bind(&chat_server::handle_accept, this, new_session, boost::asio::placeholders::error));
}

void chat_server::handle_accept(chat_session_ptr session, const boost::system::error_code& error)
{
    if (!error)
        session->start();

    start_accept();
}

void UpdateAssynMessagesThread()
{
    while(true)
    {
        sServerGlobals->Update();
        Sleep(100);
    }
}