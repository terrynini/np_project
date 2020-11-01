#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <utility>

using namespace std;
using namespace boost::asio;

void print_panel(shared_ptr<ip::tcp::socket>);
class shellSession : public enable_shared_from_this<shellSession> {
private:
    enum { max_length = 1024 };
    ip::tcp::endpoint _ep;
    ip::tcp::socket _socket;
    shared_ptr<ip::tcp::socket> _browser;
    array<char, max_length> _data;

public:
    std::string server;
    std::string inputfile;
    std::string tag;
    std::string port;
    std::fstream commands;
    shellSession(ip::tcp::endpoint ep, boost::asio::io_service& io_service, shared_ptr<ip::tcp::socket> browser)
        : _ep(move(ep))
        , _socket(io_service)
        , _browser(browser)
    {
    }
    void connect()
    {
        auto self(shared_from_this());
        commands.open("./test_case/" + inputfile, std::fstream::in);
        _socket.async_connect(_ep, [this, self](const boost::system::error_code& error) {
            if (!error)
                this->do_read();
        });
    }

private:
    void do_read()
    {
        auto self(shared_from_this());
        _socket.async_read_some(
            buffer(_data, max_length),
            [this, self](boost::system::error_code ec, size_t length) {
                if (!ec) {
                    std::string message = std::string(_data.data(), length);
                    boost::replace_all(message, "\n", "&NewLine;");
                    boost::replace_all(message, "\"", "\\\"");
                    boost::replace_all(message, "\'", "\\\'");
                    boost::replace_all(message, "<", "&#60;");
                    boost::replace_all(message, ">", "&#62;");
                    message = "<script>document.getElementById('" + tag + "').innerHTML += '" + message + "';</script>";
                    this->_browser->async_send(buffer(message.data(), message.length()), [](boost::system::error_code ec, size_t /* length */) {});
                    if (message.find("% ") == string::npos) {
                        do_read();
                    } else {
                        do_write();
                        do_read();
                    }
                }
            });
    }

    void do_write()
    {
        std::string cmd;
        if (!getline(commands, cmd))
            return;
        cmd += "\n";
        auto self(shared_from_this());
        _socket.async_send(
            buffer(cmd, cmd.length()),
            [this, self, cmd](boost::system::error_code ec, size_t /* length */) {
                if (!ec) {
                    std::string msg = cmd;
                    boost::replace_all(msg, "\n", "&NewLine;");
                    boost::replace_all(msg, "\r", "");
                    boost::replace_all(msg, "\"", "\\\"");
                    boost::replace_all(msg, "\'", "\\\'");
                    boost::replace_all(msg, "<", "&#60;");
                    boost::replace_all(msg, ">", "&#62;");
                    msg = "<script>document.getElementById('" + tag + "').innerHTML += '<b>" + msg + "</b>';</script>";
                    this->_browser->async_send(buffer(msg.data(), msg.length()), [](boost::system::error_code ec, size_t /* length */) {});
                }
            });
    }
};

extern char** environ;
io_service global_io_service;

class HttpSession : public enable_shared_from_this<HttpSession> {
private:
    enum {
        max_length = 1024
    };
    shared_ptr<ip::tcp::socket> _socket;
    array<char, max_length> _data;
    map<string, string> _env;
    std::vector<shared_ptr<shellSession>> slist;
    std::map<std::string, std::string> GET_;

public:
    HttpSession(shared_ptr<ip::tcp::socket> socket)
        : _socket(socket)
    {
        slist.clear();
        GET_.clear();
    }

    void start() { do_read(); }

private:
    void do_read()
    {
        auto self(shared_from_this());
        _socket->async_read_some(
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
        if (_env["REQUEST_METHOD"] != "GET") {
            std::string message("HTTP/1.1 418\r\n\r\nNO I'm a teapot\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN000KKKKKXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNx;,:ccccoocl0WMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWd...;:\'..,oc.;0MMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMK;...;c\'...\'\'..lNMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMK;.............cXMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWO;...........c0WMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMWNXKOOKNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWWWNNNNXXXXXKd. ... ....:0XXXNNNWWWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMWXOxxxoc;;colxXWMMMMMMMMMMMMMMMMMMMMMMMMMMWNXKKKKKKK000000000Od;..........\'oO0000KK00KXXXXK00XWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMW0o;,,;\'...:c;cOWMMMMMMMMMMMMMMMMMMMMMMMNX0xxkkkkkkxxxxxxxxxdd:...\'.....\'..,oxxkkkkOOOO0000OdloOXWMMMMMMMMMMMMMMMMMMMMMWNXXXXXNNWMMMMMMMMMMMMMMM\nMMMMMMMMW0d:;,....\',.,dNMMMMMMMMMMMMMMMMMMMMMNkoxl:;;;:clooddddxxxdddoooolcccldolodddddddddooooooddc:oO0NMMMMMMMMMMMMMMMMMNKkxdolllodxO0XWMMMMMMMMMMMM\nMMMMMMMMMMNkc,\'........oNMMMMMMMMMMMMMMMMMMMMWk;\'\'\'\'\',,;:ccllooddxkkkkkOOOOOOOOOOOOOOOOOkxolc:;,,;;;:cldKMMMMMMMMMMMMMMWXkc:::;,\'\'\'\'\',:cldONMMMMMMMMMM\nMMMMMMMMMMMWOc\'........\'xWMMMMMMMMMMMMMMMMWNXKkc\'.....\',,,\'\'\',,;clooollccccccccccclodxxxdocc:;,,,,\'...,dKWMMMMMMMMMMMMNk:\'\'\',:okO0KOxl,.,;:dKWMMMMMMMM\nMMMMMMMMMMMMWk;.........,OWMMMMMMMMMMMMWNK0Oxd:................\',;;;;;,,\'\'\'\'\'\'\'\'\',,\',;;;;;;,\'.........,okKXNWMMMMMMMW0c\'...:xXWMMMMMMWKd,.\'\':OWMMMMMMM\nMMMMMMMMMMMMMNo\'.........cXMMMMMMMMMWNKOkxdool:\'.\'\'\'.................................................\':oxkO0KXNWMMWXd,\'\'.\'lKWMMMMMMMMMMWKc...,kWMMMMMM\nMMMMMMMMMMMMMMk,..........xWMMMMMMWX0kdolcc:;,;;coddooc;,\'\'...\',;::;;,\'.............\';ccclcc:,;:loddl;;:loxkkOO0KKkc,,...lXMMMMMMMMMMMMMMXc...;0MMMMMM\nMMMMMMMMMMMMMM0;..........:KMMMMMN0xoc;,\'\'...,coooddddddoollcccclllcc::;,,,,,,,,,,;;:oddxxddddxkkOkkkxoc;;:lodxxddl;,...;0MMMMMMMMMMMMMMMM0,...oNMMMMM\nMMMMMMMMMMMMMMK:...........dNMMWXxl:,........\':lllooooddddxxxkkkkkkxxxxxxxxxxdxxxxxxxkkOOOOOOkkkkkkkkkkxdl:;;:c;,;::\'...lXMMMMMMMMMMMMMMMMNl...;0MMMMM\nMMMMMMMMMMMMMMXc........;,.,xXXOc,.............;:cclllloooodddddxxxxxxkkkkkkkkkkkkkkkkkkkkkkkxxxxxxxxxxdoc;\'.\'\'..\',,....oNMMMMMMMMMMMMMMMMMk...\'OMMMMM\nMMMMMMMMMMMMMMNl....\'\'..\':,,odl,.................,:::cccclllloooooddddddxxxxxxxxxxxxxxxxdxddddddddddol:,\'.......\',;:;,..dWMMMMMMMMMMMMMMMMMO\'..\'kMMMMM\nMMMMMMMMMMMMMMWo.....:;...,cdc,....................\',,,,,\'\'\'\'\'\',,,,,,,,,,;;;;;;;:::ccccllloooooool:;,.............\';:lll0MMMMMMMMMMMMMMMMMMO\'..,kMMMMM\nMMMMMMMMMMMMMMWd.....\'od:..\',............................................................\'\',;::;\'...................,:ld0WMMMMMMMMMMMMMMMMMO\'..,OMMMMM\nMMMMMMMMMMMMMMMk......;xko,..........................................................................................,:lkXMMMMMMMMMMMMMMMMWx...:KMMMMM\nMMMMMMMMMMMMMMMO,......,oko,..........................................................................................,cdKWMMMMMMMMMMMMMMMNl..\'lNMMMMM\nMMMMMMMMMMMMMMMK:........:c;..........................................................................................\':oOWMMMMMMMMMMMMMMMO,..,kMMMMMM\nMMMMMMMMMMMMMMMNo......................................................................................................;lkNMMMMMMMMMMMMMMXl..\'cKMMMMMM\nMMMMMMMMMMMMMMMWO;.....................................................................................................\'cxXMMMMMMMMMMMMMNd...;kWMMMMMM\nMMMMMMMMMMMMMMMMXo,\'\'\'\'\'................................................................................................:xXMMMMMMMMMMMMNo...,xNMMMMMMM\nMMMMMMMMMMMMMMMMMKo;;,,,,,,,............................................................................................:xXMMMMMMMMMMWKl...,dNMMMMMMMM\nMMMMMMMMMMMMMMMMMWKdc:;;;;;;.\',\'........................................................................................;kNMMMMMMMMMXx;...;xNMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMNkl::;;;;,,;,\'.......................................................................................,kWMMMMMMWXk:...\'cOWMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMWXOdc::;;:c;,\'\'.....................................................................................,OMMMWWX0d;....;dXMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMNKOxoodo:;,,\'\'\'\'..............\'\'\'\'\'..............................................................,kK0Oxo:\'....;o0WMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMWNXKkl:;;,,\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\',,,,,,,,\'\'\'...............\'\'\'\'..\'\'...\'\'.....................,\'..::,\'.....\':dKWMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMXxl:;;,,,,,,,\'\'\'\'\'\'\'\'\'\',,,,,,;;;;;,,,,,\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'...............,;\'......\',,:oOXWMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMXxlc:;;;,,,,,,,,,,,,,,,,;;;;;;;;;;;;,,,,\'\',,,,,\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'.............,:c,....\';cdOXWMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMXklc:;;;;;;;;;,,,,,,,,;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,\',,,,,,,,,,\'\'\'\'...........\',cddllodk0XWMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMNOoc:;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,,,,,,,,,,,,\'\'\'.........\',;lxKNNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWKxl::;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,,,,,,,,,,,\'\'\'\'\'\'\'\'\'\',,;:o0NMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMW0dc::;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,,,,,,,,,,,\'\'\'\',,,,;;:oOXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMN0xl::;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,,,,,,,,,,,,,,,,,,,,,,,,;;;;:coOXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWXkoc:;;;;,,;;;;;;;;;;;;;;;;;;;;;;;;;;;;;,,,,,,;;;;;,,,,;;;;;;:::clx0NWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWXOdc::;;;,,,,,;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;:::::coxOXWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWX0koc:;:::;;;;;;;;;;;;;;;;;::::::::::::::::::::::::::c:cldk0XNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWNXK0kdlc:;;;;;;;;;;;;;;;;;;::::::::::::::::::::;;;;:ldxOKNNWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWWNNXK0kxdolc::;;,,,\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\'\',,,;;:clodk0KNNWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWWWNNNXXKK0OkkxxddoooooooooodddxxkkO00KXNWWWWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMWWWWWWWWWWWWWWWWWWWWWWWWWWWWWMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM\nMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMMM");
            do_write(message, message.length());
            _socket->close();
        } else {
            std::string message("HTTP/1.1 200 OK\r\nContent-type: text/html\r\n\r\n");
            do_write(message, message.length());
            if (_env["REQUEST_URI"] == "/console.cgi") {
                shell_main(_socket);
            } else {
                //if(_env["REQUEST_URI"] == "/panel.cgi"){
                print_panel(_socket);
            }
        }
    }
    void shell_main(shared_ptr<ip::tcp::socket> socket)
    {
        ip::tcp::resolver resolver(global_io_service);
        ip::tcp::resolver::iterator end;
        if (_env["QUERY_STRING"] != "") {
            std::string query(_env["QUERY_STRING"]);
            std::vector<std::string> parameter;
            boost::split(parameter, query, [](char c) { return c == '&'; });
            for (auto i : parameter) {
                std::vector<std::string> temp;
                boost::split(temp, i, [](char c) { return c == '='; });
                GET_[temp[0]] = temp[1];
            }
            for (int i = 0; i < 5; i++) {
                if (GET_["h" + to_string(i)] != "" && GET_["p" + to_string(i)] != "" && GET_["f" + to_string(i)] != "") {
                    ip::tcp::resolver::iterator ei = resolver.resolve(ip::tcp::resolver::query(GET_["h" + to_string(i)], GET_["p" + to_string(i)]));
                    if (ei != end) {
                        shared_ptr<shellSession> add = make_shared<shellSession>(move(*ei), global_io_service, socket);
                        slist.push_back(add);
                        add->server = GET_["h" + to_string(i)];
                        add->inputfile = GET_["f" + to_string(i)];
                        add->tag = "s" + to_string(slist.size() - 1);
                        add->port = GET_["p" + to_string(i)];
                        add->connect();
                    }
                }
            }
        }
        htmlbody(socket);
    }

    void do_write(std::string response, size_t length)
    {
        auto self(shared_from_this());
        _socket->async_send(
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
        setenv("SERVER_PROTOCOL", "HTTP/1.1"); //now[2]);
        setenv("REMOTE_ADDR", _socket->remote_endpoint().address().to_string());
        setenv("REMOTE_PORT", to_string(_socket->remote_endpoint().port()));
        setenv("SERVER_ADDR", _socket->local_endpoint().address().to_string());
        setenv("SERVER_PORT", to_string(_socket->local_endpoint().port()));
        std::vector<std::string> temp;
        boost::split(temp, now[1], [](char c) { return c == '?'; });
        setenv("REQUEST_URI", temp[0]);
        setenv("QUERY_STRING", temp.size() > 1 ? temp[1] : "");
        request.erase(request.begin());
        for (auto line : request) {
            auto idx = line.find_first_of(":");
            if (idx != std::string::npos) {
                setenv("HTTP_" + line.substr(0, idx), line.substr(idx + 1));
            }
        }
    }

    void setenv(std::string name, std::string value)
    {
        boost::to_upper(name);
        boost::replace_all(name, "-", "_");
        boost::algorithm::trim(value);
        //::setenv(name.data(), value.data(), 1);
        _env[name] = value;
    }
    void htmlbody(shared_ptr<ip::tcp::socket> socket)
    {
        string message;
        message = R"(
<!DOCTYPE html>
<html lang="en">
  <head>
    <meta charset="UTF-8" />
    <title>NP Project 3 Console</title>
    <link
      rel="stylesheet"
      href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css"
      integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO"
      crossorigin="anonymous"
    />
    <link
      href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
      rel="stylesheet"
    />
    <link
      rel="icon"
      type="image/png"
      href="https://cdn0.iconfinder.com/data/icons/small-n-flat/24/678068-terminal-512.png"
    />
    <style>
      * {
        font-family: 'Source Code Pro', monospace;
        font-size: 1rem !important;
      }
      body {
        background-color: #212529;
      }
      pre {
        color: #cccccc;
      }
      b {
        color: #ffffff;
      }
    </style>
  </head>
  <body>
    <table class="table table-dark table-bordered">
      <thead>
        <tr>)";
        for (auto s : slist)
            message += R"(<th scope = "col"> )" + s->server + " : " + s->port + " </th >";
        message += R"(
        </tr>
      </thead>
      <tbody>
        <tr>)";
        for (auto s : slist)
            message += R"(<td><pre id = ")" + s->tag + R"(" class = "mb-0"></pre></td>)";
        message += R"(
        </tr>
      </tbody>
    </table>
     </body>
    </html>
)";
        socket->async_send(buffer(message, message.length()), [](boost::system::error_code ec, size_t /* length */) {});
    }
};

class HttpServer {
private:
    ip::tcp::acceptor _acceptor;
    ip::tcp::socket _socket;

public:
    HttpServer(short port)
        : _acceptor(global_io_service, ip::tcp::endpoint(ip::tcp::v4(), port))
        , _socket(global_io_service)
    {
        do_accept();
    }

private:
    void do_accept()
    {
        _acceptor.async_accept(_socket, [this](boost::system::error_code ec) {
            if (!ec) {
                shared_ptr<ip::tcp::socket> __socket = make_shared<ip::tcp::socket>(move(_socket));
                make_shared<HttpSession>(__socket)->start();
            }
            do_accept();
        });
    }
};

int main(int argc, char* const argv[])
{
    if (argc != 2) {
        cerr << "Usage:" << argv[0] << " [port]" << endl;
        return 1;
    }

    try {
        unsigned short port = atoi(argv[1]);
        HttpServer server(port);
        global_io_service.run();
    } catch (exception& e) {
        cerr << "Exception: " << e.what() << "\n";
    }
    return 0;
}

void print_panel(shared_ptr<ip::tcp::socket> socket)
{
    std::string message;
    std::string test_case_menu = "";
    std::string host_menu = "";
    for (int i = 0; i < 10; i++)
        test_case_menu += "<option value=\"t" + to_string(i + 1) + ".txt\">t" + to_string(i + 1) + ".txt</option>";
    for (int i = 0; i < 12; i++)
        host_menu += "<option value=\"nplinux" + to_string(i + 1) + ".cs.nctu.edu.tw\">nplinux" + to_string(i + 1) + "</option>";
    message = R"(
 <!DOCTYPE html>
 <html lang="en">
   <head>
     <title>NP Project 3 Panel</title>
     <link
       rel="stylesheet"
       href="https://stackpath.bootstrapcdn.com/bootstrap/4.1.3/css/bootstrap.min.css"
       integrity="sha384-MCw98/SFnGE8fJT3GXwEOngsV7Zt27NXFoaoApmYm81iuXoPkFOJwJ8ERdknLPMO"
       crossorigin="anonymous"
     />
     <link
       href="https://fonts.googleapis.com/css?family=Source+Code+Pro"
       rel="stylesheet"
     />
     <link
       rel="icon"
       type="image/png"
       href="https://cdn4.iconfinder.com/data/icons/iconsimple-setting-time/512/dashboard-512.png"
     />
     <style>
       * {
         font-family: 'Source Code Pro', monospace;
       }
     </style>
   </head>
   <body class="bg-secondary pt-5">)";

    message += R"(
     <form action="console.cgi" method="GET">
       <table class="table mx-auto bg-light" style="width: inherit">
         <thead class="thead-dark">
           <tr>
             <th scope="col">#</th>
             <th scope="col">Host</th>
             <th scope="col">Port</th>
             <th scope="col">Input File</th>
           </tr>
         </thead>
         <tbody>)";

    for (int i = 0; i < 5; i++) {
        message += R"(
               <tr>
                 <th scope="row" class="align-middle">Session )"
            + to_string(i + 1) + R"(</th>
                 <td>
                   <div class="input-group">
                     <select name="h)"
            + to_string(i) + R"(" class="custom-select">
                       <option></option>)"
            + host_menu + R"(
                     </select>
                     <div class="input-group-append">
                       <span class="input-group-text">.cs.nctu.edu.tw</span>
                     </div>
                   </div>
                 </td>
                 <td>
                   <input name="p)"
            + to_string(i) + R"(" type="text" class="form-control" size="5" />
                 </td>
                 <td>
                   <select name="f)"
            + to_string(i) + R"(" class="custom-select">
                     <option></option>)"
            + test_case_menu +
            R"(</select>
                 </td>
               </tr>)";
    }
    message += R"(
              <tr>
                <td colspan="3"></td>
                <td>
                  <button type="submit" class="btn btn-info btn-block">Run</button>
                </td>
              </tr>
            </tbody>
          </table>
        </form>
      </body>
    </html>)";
    socket->async_send(buffer(message, message.length()), [](boost::system::error_code ec, size_t /* length */) {});
}