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

io_service global_io_service;
char **envp;

class HttpSession : public enable_shared_from_this<HttpSession>
{
private:
    enum
    {
        max_length = 1024
    };
    ip::tcp::socket _socket;
    array<char, max_length> _data;

public:
    HttpSession(ip::tcp::socket socket) : _socket(move(socket)) {}

    void start() { do_read(); }

private:
    void do_read()
    {
        auto self(shared_from_this());
        _socket.async_read_some(
            buffer(_data, max_length),
            [this, self](boost::system::error_code ec, size_t length) {
                if (!ec)
                    parseHttp();
            });
    }
    void parseHttp()
    {
        std::vector<std::string> lines;
        std::vector<std::string> now;
        iter_split(lines, _data, boost::algorithm::first_finder("\r\n"));
        split(now, lines[0], boost::is_space());
        if (now[0] != "GET")
        {
            std::string message("HTTP/1.1 404\r\n\r\n");
            do_write(message, message.length());
            _socket.close();
        }
        else
        {
            std::string message("HTTP/1.1 200 OK\r\n");
            do_write(message, message.length());
            int pos = now[1].find("?");
            string uri = now[1][0] == '/' ? now[1].substr(1) : now[1];
            if (pos > 0)
                uri = uri.substr(0, pos - 1);
            string query = pos > 0 ? now[1].substr(pos) : "";
            do_fork(uri, query);
        }
    }

    void do_fork(string cgi, string query)
    {
        if (cgi == "")
            cgi = "panel.cgi";

        if (cgi.find("cgi") == string::npos)
            _socket.close();

        int pid = fork();
        if (pid > 0)
        {
            _socket.close();
        }
        else
        { //TODO: handle fork < 0
            int fd = _socket.native_handle();
            dup2(fd, 0);
            dup2(fd, 1);
            dup2(fd, 2);
            close(fd);
            char *argv[] = {const_cast<char *>(cgi.c_str()), NULL};
            setenv("QUERY_STRING", query.c_str(), 1);
            execv(cgi.c_str(), argv);
            // ---
            cout << " FAIL ";
        }
    }

    void do_write(std::string response, size_t length)
    {
        auto self(shared_from_this());
        _socket.async_send(
            buffer(response, length),
            [this, self](boost::system::error_code ec, size_t /* length */) {
                if (!ec)
                    do_read();
            });
    }
};

class HttpServer
{
private:
    ip::tcp::acceptor _acceptor;
    ip::tcp::socket _socket;

public:
    HttpServer(short port)
        : _acceptor(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port)),
          _socket(global_io_service)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        _acceptor.async_accept(_socket, [this](boost::system::error_code ec) {
            if (!ec)
                make_shared<HttpSession>(move(_socket))->start();
            do_accept();
        });
    }
};

int main(int argc, char *const argv[], char **envp)
{
    ::envp = envp;
    if (argc != 2)
    {
        cerr << "Usage:" << argv[0] << " [port]" << endl;
        return 1;
    }

    try
    {
        unsigned short port = atoi(argv[1]);
        HttpServer server(port);
        global_io_service.run();
    }
    catch (exception &e)
    {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}