#include <arpa/inet.h>
#include <boost/algorithm/string/replace.hpp>
#include <boost/xpressive/xpressive.hpp>
#include <fstream>
#include <iostream>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define MAXSIZE 10000
#define IS_SOCKS4A(x) (!(x & 0x00ffffff)) && (x & 0xff000000)
#define REJECT 91
#define ACCEPT 90
using namespace std;
using namespace boost::xpressive;

class Socks4 {
public:
    u_char vn;
    u_char cd;
    u_int dst_port;
    u_int dst_ip;
    char* user_id;
    u_char* request;
    int client_fd;
    char dst_ip_cstr[INET_ADDRSTRLEN];
    char ip_cstr[INET_ADDRSTRLEN];
    sockaddr_in client_addr;
    u_char response[8];
    Socks4(int client_fd, sockaddr_in client_addr);
    ~Socks4();
    int Read(int, u_char*&);
    void Parse();
    void FireWall();
    void Info();
    void ConnectMode();
    void BindMode();
    void Run();
};

class Server {
public:
    int sock;
    Server(int port);
    ~Server();
    static int genSock(int port);
    static int genClient(char* ip, int port);
    void Run();
};

int main(int argc, char const* argv[])
{
    if (argc < 2) {
        throw "Usage: ./socks_server [port]";
    }
    signal(SIGCLD, SIG_IGN);
    Server server(atoi(argv[1]));
    try {
        server.Run();
    } catch (const char* msg) {
        cout << "[ ERROR ] " << msg << endl;
    } catch (string msg){
        cout << "[ ERROR ] " << msg << endl;
    }
    return 0;
}

void Socks4::Run()
{
    Parse();
    FireWall();
    Info();
    if (cd == 1) {
        ConnectMode();
    } else if (cd == 2) {
        BindMode();
    }
}

Socks4::Socks4(int client_fd, sockaddr_in client_addr)
{
    this->client_fd = client_fd;
    this->client_addr = client_addr;
    memset(this->response, 0, 8);
}

Socks4::~Socks4()
{
    free(this->request);
}

int Socks4::Read(int fd, u_char*& result)
{
    u_char request[MAXSIZE];
    int bytes = read(fd, request, MAXSIZE);
    if (bytes == -1)
        throw "Client connection fail";
    if (bytes == 0) {
        throw "Client disconnect";
    }
    result = (u_char*)malloc(bytes);
    memcpy(result, request, bytes);
    return bytes;
}

void Socks4::Parse()
{
    Read(client_fd, request);
    vn = request[0];
    cd = request[1];
    dst_port = request[2] << 8 | request[3];
    dst_ip = ((u_int*)request)[1];
    user_id = (char*)request + 8;
    if (vn != 4) {
        throw "Unsupport socks version: " + to_string(vn);
    }
    if (IS_SOCKS4A(dst_ip)) {
        size_t user_id_len = strlen(user_id);
        char* domain_name = user_id + user_id_len + 1;
        struct hostent* phe = gethostbyname(domain_name);
        if (phe) {
            struct sockaddr_in sin;
            memcpy((char*)&sin.sin_addr, phe->h_addr, phe->h_length);
            inet_ntop(AF_INET, &sin.sin_addr, dst_ip_cstr, INET_ADDRSTRLEN);
            dst_ip = *((u_int*)phe->h_addr);
        } else {
            throw "Can not resolve domain name:" + string(domain_name);
        }
    } else {
        inet_ntop(AF_INET, &dst_ip, dst_ip_cstr, INET_ADDRSTRLEN);
    }
    inet_ntop(AF_INET, &client_addr.sin_addr, ip_cstr, INET_ADDRSTRLEN);
}

void Socks4::FireWall()
{
    string grant, mode, rule, dst_ip(dst_ip_cstr), cmode;
    smatch w;
    ifstream fin("socks.conf");
    response[1] = REJECT;
    if (fin) {
        cmode = (cd == 1 ? "c" : "b");
        while (fin >> grant >> mode >> rule) {
            boost::algorithm::replace_all(rule, ".", "\\.");
            boost::algorithm::replace_all(rule, "*", ".*");
            sregex re = sregex::compile(rule, regex_constants::icase);
            if ((mode == cmode) && regex_match(dst_ip, w, re))
                response[1] = (grant == "permit" ? ACCEPT : REJECT);
        }
    } else {
        response[1] = ACCEPT;
    }
    if (response[1] == REJECT) {
        memcpy(response + 2, request + 2, 6);
        write(client_fd, response, 8);
        Info();
        throw "FireWall: Connection reject";
    }
}

void Socks4::Info()
{
    cout << "<S_IP>: " << ip_cstr << endl
         << "<S_PORT>: " << ntohs(client_addr.sin_port) << endl
         << "<D_IP>: " << dst_ip_cstr << endl
         << "<D_PORT>: " << dst_port << endl
         << "<Command>: " << ((cd == 1) ? "CONNECT" : "BIND") << endl
         << "<Reply>: " << ((response[1] == ACCEPT) ? "Accept" : "Reject") << endl
         << endl;
}

void Socks4::ConnectMode()
{
    memcpy(response + 2, request + 2, 6);
    write(client_fd, response, 8);
    int target_fd = Server::genClient(dst_ip_cstr, dst_port);
    int maxfd_num = max(client_fd, target_fd) + 1;
    fd_set activate, fdset;
    FD_ZERO(&fdset);
    FD_SET(target_fd, &fdset);
    FD_SET(client_fd, &fdset);
    u_char* buffer;
    while (true) {
        memcpy(&activate, &fdset, sizeof(activate));
        if (select(maxfd_num, &activate, NULL, NULL, NULL)) {
            if (FD_ISSET(client_fd, &activate)) {
                write(target_fd, buffer, Read(client_fd, buffer));
            }
            if (FD_ISSET(target_fd, &activate)) {
                write(client_fd, buffer, Read(target_fd, buffer));
            }
        }
    }
}

void Socks4::BindMode()
{
    fd_set activate, fdset;
    int bind_fd = Server::genSock(0);
    struct sockaddr_in bind_addr;
    u_int bind_addr_len = sizeof(bind_addr);
    getsockname(bind_fd, (struct sockaddr*)&bind_addr, &bind_addr_len);
    auto sport = ntohs(bind_addr.sin_port);
    response[2] = sport >> 8;
    response[3] = sport & 0xff;
    memset(response + 4, 0, 4);
    write(client_fd, response, 8);
    struct sockaddr_in ftp_addr;
    u_int ftp_addr_len = sizeof(ftp_addr);
    int ftp_fd = accept(bind_fd, (struct sockaddr*)&ftp_addr, &ftp_addr_len);
    if (dst_ip != ftp_addr.sin_addr.s_addr) {
        response[1] = 91;
        write(client_fd, response, 8);
        char ftp_ip_cstr[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &ftp_addr.sin_addr, ftp_ip_cstr, INET_ADDRSTRLEN);
        throw "Wrong client, income: " + to_string(dst_ip) + " expected: " + to_string(ftp_addr.sin_addr.s_addr);
    }
    write(client_fd, response, 8);
    FD_ZERO(&fdset);
    int maxfd_num = max(client_fd, ftp_fd) + 1;
    FD_SET(ftp_fd, &fdset);
    FD_SET(client_fd, &fdset);
    u_char* buffer;
    int limit = 512*1024*1024;
    int now = 0;
    while (true) {
        memcpy(&activate, &fdset, sizeof(activate));
        if (select(maxfd_num, &activate, NULL, NULL, NULL) > 0) {
            if (FD_ISSET(client_fd, &activate)) {
                write(ftp_fd, buffer, Read(client_fd, buffer));
            }
            if (FD_ISSET(ftp_fd, &activate)) {
                if(now > limit){
                    throw "Limit exceed";
                }
                now += write(client_fd, buffer, Read(ftp_fd, buffer));
            }
        }
    }
}

Server::Server(int port)
{
    this->sock = genSock(port);
}

Server::~Server()
{
    close(this->sock);
}

void Server::Run()
{
    struct sockaddr_in client_sin;
    int client_sock;
    int socklen;
    while (true) {
        socklen = sizeof(client_sin);
        client_sock = accept(sock, (struct sockaddr*)&client_sin, (socklen_t*)&socklen);
        if (client_sock < 0) {
            throw "accept() failed";
        }
        int pid;
        while ((pid = fork()) < 0) {
            usleep(100);
        }
        if (pid) {
            close(client_sock);
        } else {
            Socks4 service(client_sock, client_sin);
            service.Run();
            break;
        }
    }
}

int Server::genSock(int port)
{
    struct sockaddr_in sin;
    int opt_val = 1;
    int sock;
    memset((char*)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = INADDR_ANY;
    if(port == 0){
        sin.sin_port = htons(INADDR_ANY);
    }else{
    sin.sin_port = htons(port); // use port 0 and bind will use the next available high port.
    }
    // sin.sin_addr.s_addr = inet_addr("127.0.0.1");
    sock = socket(AF_INET, SOCK_STREAM, 0);
    setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &opt_val, sizeof(opt_val));
    if (bind(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
        throw "getSock: can not bind on port " + to_string(port);
    if (listen(sock, 30) < 0)
        throw "getSock: can not listen on port" + to_string(port);
    return sock;
}

int Server::genClient(char* ip, int port)
{
    struct sockaddr_in sin;
    int sock;
    memset((char*)&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons((u_short)port);
    sin.sin_addr.s_addr = inet_addr(ip);
    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (connect(sock, (struct sockaddr*)&sin, sizeof(sin)) < 0)
        throw "getClient: connection fail " + string(ip) + ":" + to_string(port);
    return sock;
}