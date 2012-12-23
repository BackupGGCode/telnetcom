#pragma once

void SetUpSerialSystem(boost::asio::io_service& io_service);

class SerialPort
{
public:
    SerialPort(boost::asio::io_service& io_service, uint32 baud, const std::string& device) : active_(true), io_service_(io_service), serialPort(io_service, device)
    {
        if (!serialPort.is_open())
        {
            std::cerr << "Fallo al abrir el puerto serie.\n";
            return;
        }

        boost::asio::serial_port_base::baud_rate baud_option(baud);
        serialPort.set_option(baud_option); // establece el baudrate luego de que el puerto a sido abierto
        read_start();

        std::cout << "SerialPort> Conectado y escuchando el puerto: " << device << ", baudrate: " << baud << "." << std::endl;
    }

    void write(const char msg) // pasa los datos a escribir a la funcion do_write via el io service que corre en otro thread
    {
        io_service_.post(boost::bind(&SerialPort::do_write, this, msg));
    }

    void close() // llama a la funcion do_close via el io service que corre en otro thread
    {
        io_service_.post(boost::bind(&SerialPort::do_close, this, boost::system::error_code()));
    }

    bool active() // retorna true si el socket sigue operativo
    {
        return active_;
    }

private:

    static const int32 max_read_length = 512; // maxima cantidad de datos a leer en una operacion

    void read_start(void) ;
    void read_complete(const boost::system::error_code& error, size_t bytes_transferred);

    void do_write(const char msg);
    void write_start(void);
    void write_complete(const boost::system::error_code& error) ;

    void do_close(const boost::system::error_code& error) ;

private:
    bool active_; // retorna true si el socket sigue operativo
    boost::asio::io_service& io_service_; // io service de boost donde correra esta conexion
    boost::asio::serial_port serialPort; // objeto del puerto
    char read_msg_[max_read_length]; // datos leidos del socket
    std::deque<char> write_msgs_; // buffer de datos de escritura
};

struct SerialOptions
{
    std::string device;
    uint32 baudrate;
};

class SerialPortSingleton
{
public:

    void SetSerialPort(SerialPort* SerialPort_)
    {
        mSerialPort = SerialPort_;
        Set_ = true;
    }

    void SendSerialData(std::string msg)
    {
        if (Set_)
            mSerialPort->write(*msg.c_str());
    }

    SerialPort* mSerialPort;
    SerialOptions serialOptions;
    bool Set_;
};

#define sSerialPort CSingletonT<SerialPortSingleton>::Get()