#pragma warning ( default : 4996 )
#include <iostream>
#include <vector>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/smart_ptr.hpp>
#include <boost/smart_ptr/bad_weak_ptr.hpp>
#include <boost/thread.hpp>

const std::size_t BUFFER_SIZE = 1024;

class tcp_server : public boost::enable_shared_from_this<tcp_server> {
public:
    tcp_server(boost::asio::io_context& io_, boost::asio::ip::tcp::endpoint endpoint_)
        : _io(io_),
        _endpoint(endpoint_),
        _socket(io_),
        _strand(boost::asio::make_strand(io_)), 
        _acceptor(io_)
    {
        _acceptor.open(_endpoint.protocol());
        _acceptor.set_option(boost::asio::ip::tcp::acceptor::reuse_address(true));
        _acceptor.bind(_endpoint);
    }

    ~tcp_server() {
        _accept_thread.join();
    }

    void start() {
        
        //thread 생성자는 value를 copy함으로 만약 파라미터가 참조형이라면 그것을 명시해 주어야한다.
        _accept_thread = boost::thread(boost::bind(
            &tcp_server::accept_connections,
            this->shared_from_this()
        ));
    }
    
    void write_data() {
            boost::lock_guard<boost::mutex> lock(m);
            for (auto& socket : _clients) {
                char write_buffer[] = "AAAAAAAAAAAAAAAAAAAAAAAAAAAA";
                boost::asio::async_write( 
                    socket,
                    boost::asio::buffer(write_buffer, BUFFER_SIZE),
                    boost::asio::bind_executor(
                        _strand, 
                        boost::bind(
                            &tcp_server::write_data,
                            shared_from_this()
                        )));
            }
            //std::this_thread::sleep_for(std::chrono::milliseconds(10));
        
    }

private:
    void handle_accept(
        const boost::system::error_code& error_) {
        if (boost::system::errc::success == error_)
        {
            //async_accept는 tight loop에서 돌기때문에 strand랑 sleep 같이 붙혀서 delay 줘야함.
            boost::lock_guard<boost::mutex> lock(m);
            _clients.emplace_back(_io);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            _acceptor.async_accept(
                _socket,
                boost::bind(
                    &tcp_server::handle_accept,
                    shared_from_this(),
                    boost::asio::placeholders::error));
        }
    }
    void accept_connections() {
        _acceptor.listen();
        _acceptor.async_accept(
                _socket,
                boost::bind(
                    &tcp_server::handle_accept,
                    shared_from_this(),
                    boost::asio::placeholders::error));
            std::this_thread::sleep_for(std::chrono::milliseconds(10));

        _io.run();
        //std::cout << "out" << std::endl;
    }
    void handle_write(
        const boost::system::error_code& error,
        std::size_t bytes_transferred
    ) {
    
    }
        
        ;

private:
    boost::asio::io_context& _io;
    boost::asio::ip::tcp::endpoint _endpoint;
    boost::asio::ip::tcp::socket _socket;
    std::vector<boost::asio::ip::tcp::socket> _clients;
    boost::mutex m;
    boost::thread _accept_thread;
    char buffer_[1024];
    using	strand = boost::asio::strand< boost::asio::io_context::executor_type >;
    strand			_strand;
    using	tcp_acceptor = boost::asio::ip::tcp::acceptor;
    tcp_acceptor		_acceptor;
};

int main() {
    boost::asio::io_context io;
    try {
        boost::asio::ip::tcp::endpoint ep = boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), 3001);

        auto server(boost::make_shared<tcp_server>(io, ep));
        server->start();
        std::this_thread::sleep_for(std::chrono::seconds(1));
        while (true) server->write_data();
    }
    catch (const boost::wrapexcept<boost::bad_weak_ptr>& e) {
        std::cerr << "Exception caught: " << e.what() << std::endl;
    }
    return 0;
}
