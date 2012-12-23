#include "telnetcomPCH.hpp"
#include "serialPort.hpp"
#include "tcpServer.hpp"

void SerialPort::read_start(void)
{
    // Inicia la lectura asyncrona y llama a read_complete cuando se completa o falla
    serialPort.async_read_some(boost::asio::buffer(read_msg_, max_read_length),
        boost::bind(&SerialPort::read_complete,this,boost::asio::placeholders::error,boost::asio::placeholders::bytes_transferred));
}

void SerialPort::read_complete(const boost::system::error_code& error, size_t bytes_transferred)
{
    // La operacion de lectura asyncrona ya termino o fallo y retornara el error
    if (!error)
    {
        // lectura completada, por lo tanto procesamos los datos
        //std::cout.write(read_msg_, bytes_transferred); // eco a la salida estandar

        // Los mensajes los mandamos a todos, consola incluida.
        std::string notifier_ = std::string("Read Serial: ");
        notifier_ += read_msg_;
        notifier_ += "\n\r";
        sServerGlobals->AssynSendMessageAllClients(notifier_);

        read_start(); // Inicia la espera para otra lectura asyncrona otra vez
    }
    else
        do_close(error);
}

void SerialPort::do_write(const char msg)
{
    bool write_in_progress = !write_msgs_.empty(); // hay algo que esta escribiendo actualmente?
    write_msgs_.push_back(msg); // guarda en el write buffer
    if (!write_in_progress) // si actualmente nada se esta escribiendo, entonces iniciamos a escribir
        write_start();
}

void SerialPort::write_start(void)
{
    // inicia la escritura ayncrona y llama a write_complete cuando se completa o falla
    boost::asio::async_write(serialPort, boost::asio::buffer(&write_msgs_.front(), 1),
        boost::bind(&SerialPort::write_complete, this, boost::asio::placeholders::error));
}

void SerialPort::write_complete(const boost::system::error_code& error)
{
    // La operacion de escritura asyncrona ya termino o fallo y retornara el error
    if (!error)
    { // escritura completada, a continuacion enviamos la siguiente dara para escribir
        write_msgs_.pop_front(); // borra los datos completados
        if (!write_msgs_.empty()) // si queda algo por ser escrito
            write_start(); // entonces inicia la escritura
    }
    else
        do_close(error);
}

void SerialPort::do_close(const boost::system::error_code& error)
{
    // si algo salio mal cierra sockets, borra objetos y termina el programa.
    if (error == boost::asio::error::operation_aborted)
        return;

    if (error)
        std::cerr << "Error: " << error.message() << std::endl;
    else
        std::cout << "Error: no se pudo conectar.\n";

    std::cout << "Presione enter para salir.\n";

    serialPort.close();
    active_ = false;
}


void SetUpSerialSystem(boost::asio::io_service& io_service)
{
    try
    {
        SerialPort* c = new SerialPort(io_service, sSerialPort->serialOptions.baudrate, sSerialPort->serialOptions.device);
        sSerialPort->SetSerialPort(c);
    }
    catch (std::exception& e)
    {
        std::cerr << "Exception: " << e.what() << "\n";
    }
}
