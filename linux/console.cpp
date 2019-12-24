#include <array>
#include <boost/algorithm/string.hpp>
#include <boost/asio.hpp>
#include <cstdlib>
#include <fstream>
#include <iostream>
#include <memory>
#include <unistd.h>
#include <utility>

using namespace std;
using namespace boost::asio;

boost::asio::deadline_timer timer;

class shellSession : public enable_shared_from_this<shellSession> {
private:
    enum { max_length = 1024 };
    ip::tcp::endpoint _ep;
    ip::tcp::socket _socket;
    array<char, max_length> _data;

public:
    std::string server;
    std::string inputfile;
    std::string tag;
    std::string port;
    std::fstream commands;
    shellSession(ip::tcp::endpoint ep, boost::asio::io_service& io_service)
        : _ep(move(ep))
        , _socket(io_service)
    {
    }
    void connect()
    {
        auto self(shared_from_this());
        commands.open("./test_case/" +inputfile, std::fstream::in);
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
                    cout << "<script>document.getElementById('" << tag << "').innerHTML += '" << message << "';</script>" << endl;
                    if (message.find("% ") == string::npos) {
                        do_read();
                    } else {
                        timer.expires_from_now(boost::posix_time::seconds(2));
                        timer.async_wait([this,self](boost::system::error_code ec){
                            if(!ec){
                                do_write();
                                do_read();
                            }
                        });
                    }
                }
            });
    }

    void do_write()
    {
        std::string cmd;
        if(!getline(commands,cmd))
            return;
        cmd += "\n";
        auto self(shared_from_this());
        _socket.async_send(
            buffer(cmd, cmd.length()),
            [this, self, cmd](boost::system::error_code ec, size_t /* length */) {
                if (!ec){
                    std::string msg = cmd;
                    boost::replace_all(msg, "\n", "&NewLine;");
                    boost::replace_all(msg, "\r", "");
                    boost::replace_all(msg, "\"", "\\\"");
                    boost::replace_all(msg, "\'", "\\\'");
                    boost::replace_all(msg, "<", "&#60;");
                    boost::replace_all(msg, ">", "&#62;");
                    cout << "<script>document.getElementById('" << tag << "').innerHTML += '<b>" << msg << "</b>';</script>" << endl;
                }
            });
    }
};

extern char** environ;

void htmlbody();

std::vector<shared_ptr<shellSession>> slist;
std::map<std::string, std::string> GET_;

int main(int argc, char** argv, char** envp)
{
    io_service io_service;
    timer(io_service);
    cout << "Content-type: text/html" << endl
         << endl;
    ip::tcp::resolver resolver(io_service);
    ip::tcp::resolver::iterator end;
    if(getenv("QUERY_STRING") != ""){
    std::string query(getenv("QUERY_STRING"));
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
                shared_ptr<shellSession> add = make_shared<shellSession>(move(*ei), io_service);
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
    htmlbody();
    io_service.run();
}

void htmlbody()
{
    cout << R"(
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
        cout << R"(<th scope = "col"> )" << s->server << " : " << s->port << " </th >" << endl;
    cout << R"(
        </tr>
      </thead>
      <tbody>
        <tr>)";
    for (auto s : slist)
        cout << R"(<td><pre id = ")" << s->tag << R"(" class = "mb-0"></pre></td>)" << endl;
    cout << R"(
        </tr>
      </tbody>
    </table>
     </body>
    </html>
)";
}