#include <fstream>
#include <iostream>
#include <signal.h>
#include <unistd.h>

#include "socket.hpp"

#define BUF_SIZE 60000
#define IS_SOCK4A(x) (!(x & 0x00ffffff)) && (x & 0xff000000)
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
    sockaddr_in client_addr;
    u_char replay[8];
    Socks4(int client_fd,sockaddr_in client_addr);
    ~Socks4();
    void transport(int from_fd, int to_fd);
    void Read();
};

void
socks4_handler(int client, const sockaddr_in client_adr)
{
    Socks4 service(client,client_adr);
    service.Read();
    
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
            socks4_handler(ssock, fsin);
            exit(0);
        }
    }
    return 0;
}

Socks4::Socks4(int client_fd,sockaddr_in client_addr)
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
    u_char request[BUF_SIZE];
    int read_count = read(client_fd, request, BUF_SIZE);
    if (read_count == -1)
        error("Client connection fail");
    if (read_count == 0) {
        cout << "Client disconnect" << endl;
        exit(0);
    }
    this->request = (u_char*)malloc(read_count);
    memcpy(this->request, request, read_count);
}

/* exit(0) if read eof from from_fd */
void Socks4::transport(int from_fd, int to_fd)
{
    char buffer[BUF_SIZE] = { 0 };
    int read_count = read(from_fd, buffer, BUF_SIZE);
    if (read_count == 0) { /* read() eof */
        exit(0);
    } else if (read_count == -1) {
        error("Read error");
    } else {
        int write_count = write(to_fd, buffer, read_count);
        if (write_count != read_count) {
            cout << "[Warning] from_fd -> to_fd:  write_count != read_count" << endl;
        }
    }
}