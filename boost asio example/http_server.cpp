#include <array>
#include <boost/asio.hpp>
#include <boost/algorithm/string.hpp>
#include <cstdlib>
#include <iostream>
#include <memory>
#include <map>
#include <unistd.h>
#include <utility>

using namespace std;
using namespace boost::asio;

io_service global_io_service;

class HttpSession : public enable_shared_from_this<HttpSession>
{
private:
    enum
    {
        max_length = 1024
    };
    ip::tcp::socket _socket;
    array<char, max_length> _data;
    map<string, string> _env;

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

        iter_split(lines, _data, boost::algorithm::first_finder("\r\n"));
        this->ParseHttpLines(lines);
        if (_env["REQUEST_METHOD"] != "GET")
        {
            std::string message("HTTP/1.1 418\r\n\r\nNO I'm a teapot\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN000KKKKKXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNx;,:ccccoocl0WMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWd...;:\'..,oc.;0MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMK;...;c\'...\'\'..lNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMK;.............cXMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWO;...........c0WMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMWNXKOOKNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWWWNNNNXXXXXKd. ... ....:0XXXNNNWWWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMWXOxxxoc;;colxXWMMMMMMMMMMMMMMMMMMMMMMMMMMWNXKKKKKKK000000000Od;..........\'oO0000KK00KXXXXK00XWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMW0o;,,;\'...:c;cOWMMMMMMMMMMMMMMMMMMMMMMMNX0xxkkkkkkxxxxxxxxxdd:...\'.....\'..,oxxkkkkOOOO0000OdloOXWMMMMMMMMMMMMMMMMMMMMMWNXXXXXNNWMMMMMMMMMMMMMMM\nMMMMMMMMW0d:;,....\',.,dNMMMMMMMMMMMMMMMMMMMMMNkoxl:;;;:clooddddxxxdddoooolcccldolodddddddddooooooddc:oO0NMMMMMMMMMMMMMMMMMNKkxdolllodxO0XWMMMMMMMMMMMM\nMMMMMMMMMMNkc,\'........oNMMMMMMMMMMMMMMMMMMMMWk;\'\'\'\'\',,;:ccllooddxkkkkkOOOOOOOOOOOOOOOOOkxolc:;,,;;;:cldKMMMMMMMMMMMMMMWXkc:::;,\'\'\'\'\',:cldONMMMMMMMMMM\nMMMMMMMMMMMWOc\'........\'xWMMMMMMMMMMMMMMMMWNXKkc\'.....\',,,\'\'\',,;clooollccccccccccclodxxxdocc:;,,,,\'...,dKWMMMMMMMMMMMMNk:\'\'\',:okO0KOxl,.,;:dKWMMMMMMMM\nMMMMMMMMMMMMWk;.........,OWMMMMMMMMMMMMWNK0Oxd:................\',;;;;;,,\'\'\'\'\'\'\'\'\',,\',;;;;;;,\'.........,okKXNWMMMMMMMW0c\'...:xXWMMMMMMWKd,.\'\':OWMMMMMMM\nMMMMMMMMMMMMMNo\'.........cXMMMMMMMMMWNKOkxdool:\'.\'\'\'.................................................\':oxkO0KXNWMMWXd,\'\'.\'lKWMMMMMMMMMMWKc...,kWMMMMMM\nMMMMMMMMMMMMMMk,..........xWMMMMMMWX0kdolcc:;,;;coddooc;,\'\'...\',;::;;,\'.............\';ccclcc:,;:loddl;;:loxkkOO0KKkc,,...lXMMMMMMMMMMMMMMXc...;0MMMMMM\nMMMMMMMMMMMMMM0;..........:KMMMMMN0xoc;,\'\'...,coooddddddoollcccclllcc::;,,,,,,,,,,;;:oddxxddddxkkOkkkxoc;;:lodxxddl;,...;0MMMMMMMMMMMMMMMM0,...oNMMMMM\nMMMMMMMMMMMMMMK:...........dNMMWXxl:,........\':lllooooddddxxxkkkkkkxxxxxxxxxxdxxxxxxxkkOOOOOOkkkkkkkkkkxdl:;;:c;,;::\'...lXMMMMMMMMMMMMMMMMNl...;0MMMMM\nMMMMMMMMMMMMMMXc........;,.,xXXOc,.............;:cclllloooodddddxxxxxxkkkkkkkkkkkkkkkkkkkkkkkxxxxxxxxxxdoc;\'.\'\'..\',,....oNMMMMMMMMMMMMMMMMMk...\'OMMMMM\nMMMMMMMMMMMMMMNl....\'\'..\':,,odl,.................,:::cccclllloooooddddddxxxxxxxxxxxxxxxxdxddddddddddol:,\'.......\',;:;,..dWMMMMMMMMMMMMMMMMMO\'..\'kMMMMM\nMMMMMMMMMMMMMMWo.....:;...,cdc,....................\',,,,,\'\'\'\'\'\',,,,,,,,,,;;;;;;;:::ccccllloooooool:;,.............\';:lll0MMMMMMMMMMMMMMMMMMO\'..,kMMMMM\nMMMMMMMMMMMMMMWd.....\'od:..\',............................................................\'\',;::;\'...................,:ld0WMMMMMMMMMMMMMMMMMO\'..,OMMMMM\nMMMMMMMMMMMMMMMk......;xko,..........................................................................................,:lkXMMMMMMMMMMMMMMMMWx...:KMMMMM\nMMMMMMMMMMMMMMMO,......,oko,..........................................................................................,cdKWMMMMMMMMMMMMMMMNl..\'lNMMMMM\nMMMMMMMMMMMMMMMK:........:c;..........................................................................................\':oOWMMMMMMMMMMMMMMMO,..,kMMMMMM\nMMMMMMMMMMMMMMMNo......................................................................................................;lkNMMMMMMMMMMMMMMXl..\'cKMMMMMM\nMMMMMMMMMMMMMMMWO;.....................................................................................................\'cxXMMMMMMMMMMMMMNd...;kWMMMMMM\nMMMMMMMMMMMMMMMMXo,\'\'\'\'\'................................................................................................:xXMMMMMMMMMMMMNo...,xNMMMMMMM\nMMMMMMMMMMMMMMMMMKo;;,,,,,,,............................................................................................:xXMMMMMMMMMMWKl...,dNMMMMMMMM\nMMMMMMMMMMMMMMMMMWKdc:;;;;;;.\',\'........................................................................................;kNMMMMMMMMMXx;...;xNMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMNkl::;;;;,,;,\'.......................................................................................,kWMMMMMMWXk:...\'cOWMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMWXOdc::;;:c;,\'\'.....................................................................................,OMMMWWX0d;....;dXMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMNKOxoodo:;,,\'\'\'\'..............\'\'\'\'\'..............................................................,kK0Oxo:\'....;o0WMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMWNXKkl:;;,,\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\',,,,,,,,\'\'\'...............\'\'\'\'..\'\'...\'\'.....................,\'..::,\'.....\':dKWMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMXxl:;;,,,,,,,\'\'\'\'\'\'\'\'\'\',,,,,,;;;;;,,,,,\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'...............,;\'......\',,:oOXWMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMXxlc:;;;,,,,,,,,,,,,,,,,;;;;;;;;;;;;,,,,\'\',,,,,\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'.............,:c,....\';cdOXWMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMXklc:;;;;;;;;;,,,,,,,,;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,\',,,,,,,,,,\'\'\'\'...........\',cddllodk0XWMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNOoc:;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\'\'\'.........\',;lxKNNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWKxl::;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,,,,,,,,,,,\'\'\'\'\'\'\'\'\'\',,;:o0NMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMW0dc::;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,,,,,,,,,,,\'\'\'\',,,,;;:oOXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN0xl::;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,,,,,,,,,,,,;;;;:coOXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWXkoc:;;;;,,;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,;;;;;,,,,;;;;;;:::clx0NWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWXOdc::;;;,,,,,;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;:::::coxOXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWX0koc:;:::;;;;;;;;;;;;;;;;;::::::::::::::::::::::::::c:cldk0XNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWNXK0kdlc:;;;;;;;;;;;;;;;;;;::::::::::::::::::::;;;;:ldxOKNNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWWNNXK0kxdolc::;;,,,\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\',,,;;:clodk0KNNWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWWWNNNXXKK0OkkxxddoooooooooodddxxkkO00KXNWWWWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWWWWWWWWWWWWWWWWWWWWWWWWWWWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
            do_write(message, message.length());
            _socket.close();
        }
        else
        {
            std::string message("HTTP/1.1 200 OK\r\n");
            do_write(message, message.length());
            do_fork(_env["REQUEST_URI"], _env["QUERY_STRING"]);
        }
    }

    void do_fork(string cgi, string query)
    {
        cgi = cgi.substr(1);
        //truncat "/"
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
    void ParseHttpLines(std::vector<std::string> request)
    {
        std::vector<std::string> now;
        split(now, request[0], boost::is_space());
        setenv("REQUEST_METHOD", now[0]);
        setenv("SERVER_PROTOCOL", "HTTP/1.1");//now[2]);
        setenv("REMOTE_ADDR", _socket.remote_endpoint().address().to_string());
        setenv("REMOTE_PORT", to_string(_socket.remote_endpoint().port()));
        setenv("SERVER_ADDR", _socket.local_endpoint().address().to_string());
        setenv("SERVER_PORT", to_string(_socket.local_endpoint().port()));
        std::vector<std::string> temp;
        boost::split(temp, now[1], [](char c) { return c == '?'; });
        setenv("REQUEST_URI", temp[0]);
        setenv("QUERY_STRING", temp.size() > 1 ? temp[1] : "");
        request.erase(request.begin());
        for (auto line : request)
        {
            auto idx = line.find_first_of(":");
            if( idx != std::string::npos){
                setenv( "HTTP_"+line.substr(0,idx),line.substr(idx+1));
            }
        }
    }

    void setenv(std::string name, std::string value)
    {
        boost::to_upper(name);
        boost::replace_all(name,"-","_");
        boost::algorithm::trim(value);
        ::setenv(name.data(), value.data(), 1);
        _env[name] = value;
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

int main(int argc, char *const argv[])
{
    clearenv();
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