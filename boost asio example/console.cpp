#include <array>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <utility>

using namespace std;
using namespace boost::asio;

class shellSession : public enable_shared_from_this<shellSession>
{
private:
    enum
    {
        max_length = 1024
    };
    ip::tcp::socket _socket;
    array<char, max_length> _data;

public:
    shellSession(ip::tcp::socket socket) : _socket(move(socket)) {}

private:
    void do_read()
    {
        auto self(shared_from_this());
        _socket.async_read_some(
            buffer(_data, max_length),
            [this, self](boost::system::error_code ec, size_t length) {
                if (!ec)
                    do_write(length);
            });
    }

    void do_write(size_t length)
    {
        auto self(shared_from_this());
        _socket.async_send(
            buffer(_data, length),
            [this, self](boost::system::error_code ec, size_t /* length */) {
                if (!ec)
                    do_read();
            });
    }
};

int main(int argc, char** argv, char** envp){
    cout << "Content-type: text/html" << endl << endl;
    for(int i = 0 ; envp[i] ; i++){
        cout << envp[i] <<  endl;        
    }
    cout << "endl";
}