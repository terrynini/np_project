#include <fstream>
#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "socket.hpp"

#define BUF_SIZE 60000
#define IS_SOCKS4A(x) (!(x & 0x00ffffff)) && (x & 0xff000000)
using namespace std;

class Socks4 {
public:
    u_char sp_vn;
    u_char sp_cd;
    u_int sp_dst_port;
    u_int sp_dst_ip;
    char* sp_user_id;
    u_char* request;
    int client_fd;
    char sp_dst_ip_cstr[INET_ADDRSTRLEN];
    sockaddr_in client_addr;
    u_char reply[8];
    Socks4(int client_fd, sockaddr_in client_addr);
    ~Socks4();
    void transport(int from_fd, int to_fd);
    void Read();
    void Parse();
    void FireWall();
    void Info();
    void Reject();
    void ConnectMode();
    void BindMode();
    void Run();
};

void Socks4::Run()
{
    Read();
    Parse();
    FireWall();
    Info();
    Reject();
    if (sp_cd == 1) {
        ConnectMode();
    } else if (sp_cd == 2) {
        BindMode();
    } else {
        error("Bad sock4 request, CD = " + std::to_string(sp_cd));
    }
}

int main(int argc, char const* argv[])
{
    struct sockaddr_in fsin;
    int msock, ssock;
    int alen;
    if (argc < 2) {
        error("Usage: ./socks_server [port]");
    }
    signal(SIGCLD, SIG_IGN);
    msock = server_sock(atoi(argv[1]));
    while (true) {
        alen = sizeof(fsin);
        ssock = accept(msock, (struct sockaddr*)&fsin, (socklen_t*)&alen);
        if (ssock < 0) {
            error("Accept failed: ");
        }
        int pid;
        while ((pid = fork()) < 0) {
            usleep(100);
        }
        if (pid) {
            close(ssock);
        } else {
            close(msock);
            Socks4 service(ssock, fsin);
            service.Run();
            exit(0);
        }
    }
    return 0;
}

Socks4::Socks4(int client_fd, sockaddr_in client_addr)
{
    this->client_fd = client_fd;
    this->client_addr = client_addr;
}

Socks4::~Socks4()
{
    free(this->request);
}

void Socks4::Read()
{

}

/* exit(0) if read eof from from_fd */
void Socks4::transport(int from_fd, int to_fd)
{

}

void Socks4::Parse()
{
    sp_vn = request[0];
    sp_cd = request[1];
    sp_dst_port = request[2] << 8 | request[3];
    sp_dst_ip = ((u_int*)request)[1];
    sp_user_id = (char*)request + 8;
    if (sp_vn != 4) {
        error("Bad sock4 request, VN = " + sp_vn);
    }

}

void Socks4::FireWall()
{

}

void Socks4::Info()
{

}

void Socks4::Reject()
{

}

void Socks4::ConnectMode()
{

}

void Socks4::BindMode()
{

}