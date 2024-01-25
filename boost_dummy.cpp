#pragma warning ( default : 4996 )
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>

class tcp_client : public boost::enable_shared_from_this<tcp_client> {
public:

    tcp_client(boost::asio::io_context& io_, boost::asio::ip::tcp::endpoint ep_)
        : _io(io_),
        _endpoint(ep_),
        _socket(io_),
        _strand(boost::asio::make_strand(io_))
    {
   
    }

    void start() {
        _socket.async_connect(_endpoint, [this](const boost::system::error_code& ec) {
            if (!ec) {
                std::cout << "Connected to server\n";
                // Perform any additional setup or operations after successful connection
            }
            else {
                std::cerr << "Error connecting to server: " << ec.message() << "\n";
            }
            });
        _io.run();
    }

    boost::asio::ip::tcp::socket& tcp_socket()
    {
        return	_socket;
    }

    void start_receiving() {
        if (connected)
        {
            tcp_socket().non_blocking(true);
            tcp_socket().set_option(boost::asio::ip::tcp::no_delay(true));
            tcp_socket().set_option(boost::asio::socket_base::keep_alive(true));

            boost::asio::async_read(tcp_socket(), boost::asio::buffer(buffer_),
                boost::asio::bind_executor(
                    _strand,
                    boost::bind(
                        &tcp_client::handle_receive,
                        this->shared_from_this(),
                        boost::asio::placeholders::error,
                        boost::asio::placeholders::bytes_transferred)));
        }
    }
private:
    

    void handle_connect() {
        std::cout << "Connected to server\n";
        connected = true;
    }

    //void handle_connect_error(const boost::system::error_code& ec) {
    //    std::cerr << "Error connecting to server: " << ec.message() << "\n";
    //    _io.stop();
    //}

    void handle_receive(const boost::system::error_code& ec_, std::size_t bytes_transferred_) {
        std::cout << "Received " << bytes_transferred_ << " bytes from server: " << buffer_ << "\n";
        boost::asio::async_read(tcp_socket(), boost::asio::buffer(buffer_),
            boost::asio::bind_executor(
                _strand,
                boost::bind(
                    &tcp_client::handle_receive,
                    this->shared_from_this(),
                    boost::asio::placeholders::error,
                    boost::asio::placeholders::bytes_transferred)));
    }

private:
    //using	session_ptr = boost::shared_ptr< tcp_client >;
    boost::asio::io_context& _io;
    boost::asio::ip::tcp::endpoint _endpoint;
    boost::asio::ip::tcp::socket _socket;
    
    char buffer_[1024];
    bool connected = false;
    using	strand = boost::asio::strand< boost::asio::io_context::executor_type >;
    strand			_strand;
};

int main() {
    boost::asio::io_context io;

    boost::asio::ip::tcp::endpoint ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 3001);

    //make shared : single heap allocation
    auto client = boost::make_shared<tcp_client>(io, ep);
    client->start();
    while(true) client->start_receiving();

    return 0;
}
